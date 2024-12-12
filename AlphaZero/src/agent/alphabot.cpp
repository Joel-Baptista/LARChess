#include "../headers/alphabot.h"

AlphaBot::AlphaBot(std::shared_ptr<Game> game, 
                   int num_searches, 
                   float C, 
                   float temp,
                   float dichirlet_alpha, 
                   float dichirlet_epsilon, 
                   int num_resblocks,
                   int num_channels,
                   std::string device)
{
    m_Game = game;
    m_NumSearches = num_searches;
    m_C = C;
    m_temp = temp;
    m_Device = std::make_shared<torch::Device>(torch::kCPU);

    if (torch::cuda::is_available() && (device.find("cuda") != device.npos))
    {
        auto dots = device.find(":");
        int device_id = 0;
        if (dots != device.npos)
        {
            device_id = std::stoi(device.substr(dots + 1, device.npos - dots));
        }

        m_Device = std::make_unique<torch::Device>(torch::kCUDA, device_id);
        std::cout << "Using CUDA " << device_id << std::endl;

    }
    else
    {
        std::cout << "Using CPU" << std::endl;
        m_Device = std::make_unique<torch::Device>(torch::kCPU);
    }

    m_ResNetChess = std::make_shared<ResNetChess>(num_resblocks, num_channels, *m_Device);
    m_mcts = std::make_unique<MCTS>(m_ResNetChess, 0, num_searches, dichirlet_alpha, dichirlet_epsilon, C);
}

AlphaBot::~AlphaBot()
{
}

void AlphaBot::load_model(std::string path)
{
    std::vector<int64_t> shape = {1, 19, 8, 8};
    torch::Tensor encoded_state = torch::rand(shape, torch::kFloat32); // Initialize the tensor with zeros
    encoded_state = encoded_state.to(*m_Device);
    // auto st = get_time_ms();

    // m_ResNetChess->forward(encoded_state);
    // std::cout << "Time to infer: " << ((get_time_ms() - st) / 1000.0f) << " seconds" << std::endl;

    // int negative_weight_counts = 0;
    // float min = 1000000;
    // for (auto& param : m_ResNetChess->parameters()) {
    //     float min_aux = param.data().abs().min().item<float>();
    //     if (min_aux != 0.0f) min = min_aux;
    // } 
    // std::cout << "Min: " << min << std::endl;
    // torch::save(m_ResNetChess, path + "model_aux.pt");
    torch::load(m_ResNetChess, path + "model.pt", *m_Device);
    m_ResNetChess->eval();

    // min = 100000000;

    // for (auto& param : m_ResNetChess->parameters()) {
    //     float min_aux = param.data().abs().min().item<float>();
    //     if (min_aux != 0.0f) min = min_aux;
    // } 

    // std::cout << "Min Before: " << min << std::endl;

    
    auto st = get_time_ms();
    auto output1= m_ResNetChess->forward(encoded_state);
    std::cout << "Before clamp: " << ((get_time_ms() - st) / 1000.0f) << " seconds" << std::endl;
    
    m_ResNetChess->to(*m_Device, torch::kFloat32);
    // for (auto& param : m_ResNetChess->parameters()) {
    //     param.data().clamp_(1e-6); // Clamp weights to avoid extremely small or large values
    // }

    clamp_small_weights(*m_ResNetChess, 1e-15);

    // negative_weight_counts = 0;
    // min = 10000000;
   
    // for (auto& param : m_ResNetChess->parameters()) {
    //     float min_aux = param.data().abs().min().item<float>();
    //     if (min_aux != 0.0f) min = min_aux;
    // } 
    // std::cout << "Min After: " << min << std::endl;


    st = get_time_ms();
    auto output2 = m_ResNetChess->forward(encoded_state);
    std::cout << "After clamp: " << ((get_time_ms() - st) / 1000.0f) << " seconds" << std::endl;

    if (torch::allclose(output1.policy, output2.policy) || torch::allclose(output1.value, output2.value))
    {
        std::cout << "Model loaded successfully" << std::endl;
    }
    else
    {
        std::cout << "Be carefull! Clamping the weights significantly altered the network" << std::endl;
    }

    // std::cout << "Comparing outputs" << std::endl;
    // std::cout << "Policy : " << torch::allclose(output1.policy, output2.policy) << std::endl;
    
    // std::cout << "Value: " << torch::allclose(output1.value, output2.value) << std::endl;
    // std::cout << "Value 1: " << output1.value.item<float>() << std::endl;
    // std::cout << "Value 2: " << output2.value.item<float>() << std::endl;
    // std::cout << "Policy 1 mean: " << output1.policy.mean().item<float>() << std::endl; 
    // std::cout << "Policy 1 std: " << output1.policy.std().item<float>() << std::endl;
    // std::cout << "Policy 2 mean: " << output2.policy.mean().item<float>() << std::endl; 
    // std::cout << "Policy 2 std: " << output2.policy.std().item<float>() << std::endl; 

    // std::cout << "Number of differences in policies: " << (output1.policy != output2.policy).sum().item<int>() << std::endl;
    // std::cout << "Mean of the differences in policies: " << (output1.policy - output2.policy).abs().mean().item<float>() << std::endl;
    // std::cout << "Max of the differences in policies: " << (output1.policy - output2.policy).abs().max().item<float>() << std::endl;
    // std::cout << "Min of the differences in policies: " << (output1.policy - output2.policy).abs().min().item<float>() << std::endl;


    // assert(torch::allclose(output1.policy, output2.policy));
    // assert(torch::allclose(output1.value, output2.value));
    // std::cout << "Model loaded successfully" << std::endl;
}

std::string AlphaBot::predict()
{
    std::vector<SPG*> spGames;
    SPG* spg = new SPG(m_Game);
    spGames.push_back(spg);

    auto st = get_time_ms();

    copy_alpha_board(m_Game->m_Board);

    m_mcts->search(&spGames);

    restore_alpha_board(m_Game->m_Board);
    std::cout << "Time to search: " << ((get_time_ms() - st) / 1000.0f) << " seconds" << std::endl;
    st = get_time_ms();
    std::vector<int64_t> shape = {8, 8, 73};
    torch::Tensor action_probs = torch::zeros(shape, torch::kFloat32); // Initialize the tensor with zeros

    std::vector<int> visited_childs;
    std::cout << "Moves: ["; 
    for (int j = 0; j < spGames.at(0)->pRoot->pChildren.size(); j++)
    {
        if (spGames.at(0)->pRoot->pChildren.at(j)->visit_count > 0)
        {
            visited_childs.push_back(j);
        }


        action_probs += 
            spGames.at(0)->game->get_encoded_action(spGames.at(0)->pRoot->pChildren.at(j)->action, spGames.at(0)->current_state.side) 
            * spGames.at(0)->pRoot->pChildren.at(j)->visit_count;
        
        std::cout << spGames.at(0)->pRoot->pChildren.at(j)->action << ", ";
    }
    std::cout << "]" << std::endl;
    action_probs /= action_probs.sum();

    torch::Tensor temperature_action_probs = action_probs.pow(1.0 / m_temp);
    temperature_action_probs /= temperature_action_probs.sum();

    auto temp_indexs = torch::nonzero(temperature_action_probs);

    std::vector<double> probabilities(spGames.at(0)->pRoot->pChildren.size());

    for (int idx = 0; idx < temp_indexs.sizes()[0]; idx++)
    {
        int row = temp_indexs[idx][0].item<int>();
        int col = temp_indexs[idx][1].item<int>();
        int plane = temp_indexs[idx][2].item<int>();

        probabilities[visited_childs[idx]] = temperature_action_probs[row][col][plane].item<double>();
    }

    std::cout << "Probs: [";
    for (int i=0; i < probabilities.size(); i++)
    {
        std::cout << probabilities.at(i) << ", ";
    }
    std::cout << "]" << std::endl;

    std::cout << "Evaluations: ["; 
    for (int j = 0; j < spGames.at(0)->pRoot->pChildren.size(); j++)
    {
        std::cout << (spGames.at(0)->pRoot->pChildren.at(j)->value_sum / spGames.at(0)->pRoot->pChildren.at(j)->visit_count) << ", ";
    }
    std::cout << "]" << std::endl;

    std::string  move = spGames.at(0)->game->decode_action(spGames.at(0)->current_state, action_probs); 
    std::cout << "Time to get action probs: " << ((get_time_ms() - st) / 1000.0f) << " seconds" << std::endl;

    std::cout << "Move: " << move << std::endl;
    
    return move;
}

void AlphaBot::make_bot_move(std::string move)
{
    m_Game->m_Board->make_move(m_Game->m_Board->parse_move(move.c_str()));
}