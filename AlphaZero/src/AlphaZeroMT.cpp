#include "AlphaZeroMT.h"
#include <algorithm> // For std::shuffle
#include <random>    // For std::mt19937
#include <chrono>    // For std::chrono::system_clock


AlphaZeroMT::AlphaZeroMT(
                        int num_searches, 
                        int num_iterations, 
                        int num_selfPlay_iterations, 
                        int num_parallel_games, 
                        int num_epochs, 
                        int batch_size, 
                        float temperature, 
                        float temperature_decay, 
                        float temperature_min, 
                        float learning_rate, 
                        float dichirlet_alpha, 
                        float dichirlet_epsilon, 
                        float dichirlet_epsilon_decay, 
                        float dichirlet_epsilon_min, 
                        float C,
                        float C_decay,
                        float C_min,
                        int num_evals,
                        int depth,
                        float weight_decay,
                        int num_resblocks,
                        int num_channels,
                        std::string device,
                        std::string pretrained_model_path,
                        int num_threads
                        )
{

    log_file = "log.txt";
    initLogFile(log_file);

    if (torch::cuda::is_available() && (device.find("cuda") != device.npos))
    {

        auto dots = device.find(":");
        int device_id = 0;
        if (dots != device.npos)
        {
            device_id = std::stoi(device.substr(dots + 1, device.npos - dots));
        }

        m_Device = std::make_unique<torch::Device>(torch::kCUDA, device_id);
        logMessage("Using CUDA " + std::to_string(device_id), log_file);
    }
    else
    {
        logMessage("Using CPU", log_file);
        m_Device = std::make_unique<torch::Device>(torch::kCPU);
    }
    
    m_ResNetChess = std::make_shared<ResNetChess>(num_resblocks, num_channels, *m_Device);

    if (pretrained_model_path != "")
    {
        load_model(pretrained_model_path);
    }
    
    m_Optimizer = std::make_unique<torch::optim::Adam>(m_ResNetChess->parameters(), torch::optim::AdamOptions(learning_rate).weight_decay(weight_decay));

    for (int i = 0; i < num_threads; i++)
    {
        m_ResNetSwarm.push_back(std::make_shared<ResNetChess>(num_resblocks, num_channels, *m_Device));
        copy_weights(*m_ResNetChess, *m_ResNetSwarm.at(i));
        m_mcts.push_back(std::make_unique<MCTS>(m_ResNetSwarm.at(i), num_searches, dichirlet_alpha, dichirlet_epsilon, C));
    }


    for (int i = 0; i < num_threads; i++)
    {
        games.push_back(std::make_shared<Game>());
    }

    this->num_searches = num_searches;
    this->num_iterations = num_iterations;
    this->num_selfPlay_iterations = num_selfPlay_iterations;
    this->num_parallel_games = num_parallel_games;
    this->num_epochs = num_epochs;
    this->batch_size = batch_size;
    this->temperature = temperature;
    this->temperature_decay = temperature_decay;
    this->temperature_min = temperature_min;
    this->learning_rate = learning_rate;
    this->dichirlet_alpha = dichirlet_alpha;
    this->dichirlet_epsilon = dichirlet_epsilon;
    this->dichirlet_epsilon_decay = dichirlet_epsilon_decay;
    this->dichirlet_epsilon_min = dichirlet_epsilon_min;
    this->C = C;
    this->C_decay = C_decay;
    this->C_min = C_min;
    this->num_evals = num_evals;
    this->depth = depth;
    this->weight_decay = weight_decay;
    this->num_resblocks = num_resblocks;
    this->num_threads = num_threads;

}

AlphaZeroMT::~AlphaZeroMT()
{
}

void AlphaZeroMT::update_dichirlet()
{
    dichirlet_epsilon = std::max(dichirlet_epsilon * dichirlet_epsilon_decay, dichirlet_epsilon_min);

    for (int i = 0; i < num_threads; i++)
        m_mcts.at(i)->set_dichirlet_epsilon(dichirlet_epsilon);

    logMessage("Dichirlet epsilon: " + std::to_string(dichirlet_epsilon) + " Dichirlet alpha: " + std::to_string(dichirlet_alpha), log_file);   
}
void AlphaZeroMT::update_temperature()
{
    temperature = std::max(temperature * temperature_decay, temperature_min);

    logMessage("Temperature: " + std::to_string(temperature), log_file);   
}
void AlphaZeroMT::update_C()
{
    C = std::max(C * C_decay, C_min);

    for (int i = 0; i < num_threads; i++)
        m_mcts.at(i)->set_C(C);

    logMessage("C: " + std::to_string(C), log_file);   
}

std::vector<sp_memory_item> AlphaZeroMT::SelfPlay(int thread_id)
{
    std::vector<sp_memory_item> memory;
    std::vector<SPG*> spGames;
    std::vector<int*> spg_times;

    for (int i = 0; i < num_parallel_games; i++)
    {
        games.at(thread_id)->m_Board->parse_fen(start_position);
        // games.at(thread_id)->m_Board->parse_fen("7k/5K1P/8/8/8/8/8/8 b - - 0 1");
        SPG* spg = new SPG(games.at(thread_id));
        spGames.push_back(spg);
        int* p = new int(get_time_ms());
        spg_times.push_back(p);
    }

    int count = 0;
    while (spGames.size() > 0)
    {

        auto st = get_time_ms();
        // auto st = get_time_ms();
        auto st_total = get_time_ms();

        count++;

        auto st_mcts = get_time_ms();
        m_mcts.at(thread_id)->search(&spGames);
        // std::cout << "MCTS Time: " << (float)(get_time_ms() - st_mcts) / 1000.0f << " seconds" << std::endl;
        
        for (int i = spGames.size() - 1; i >= 0; i--)
        {


            std::vector<int64_t> shape = {8, 8, 73};
            torch::Tensor action_probs = torch::zeros(shape, torch::kFloat32); // Initialize the tensor with zeros
            
            std::vector<int> visited_childs;
            // std::cout << "Visit Counts: [";
            for (int j = 0; j < spGames.at(i)->pRoot->pChildren.size(); j++)
            {
                // std::cout << spGames.at(i)->pRoot->pChildren.at(j)->visit_count << " ";
                if (spGames.at(i)->pRoot->pChildren.at(j)->visit_count > 0)
                {
                    visited_childs.push_back(j);
                }

                action_probs += 
                    spGames.at(i)->game->get_encoded_action(spGames.at(i)->pRoot->pChildren.at(j)->action, spGames.at(i)->current_state.side) 
                    * spGames.at(i)->pRoot->pChildren.at(j)->visit_count;
            }
            // std::cout << "]" << std::endl;

            action_probs /= action_probs.sum();

            spGames.at(i)->memory.push_back({spGames.at(i)->current_state, action_probs, spGames.at(i)->current_state.side});

            // std::cout << "Decode action: " << (float)(get_time_ms() - st) / 1000.0f << " seconds" << std::endl;
            // st = get_time_ms();
            
            torch::Tensor temperature_action_probs = action_probs.pow(1.0 / temperature);
            temperature_action_probs /= temperature_action_probs.sum();

            auto temp_indexs = torch::nonzero(temperature_action_probs);

            std::vector<double> probabilities(spGames.at(i)->pRoot->pChildren.size());

            for (int idx = 0; idx < temp_indexs.sizes()[0]; idx++)
            {
                int row = temp_indexs[idx][0].item<int>();
                int col = temp_indexs[idx][1].item<int>();
                int plane = temp_indexs[idx][2].item<int>();

                probabilities[visited_childs[idx]] = temperature_action_probs[row][col][plane].item<double>();
            }

            std::random_device rd;
            std::mt19937 gen(rd());
            std::discrete_distribution<> dist(probabilities.begin(), probabilities.end());
            int action_idx = dist(gen);

            std::vector<int>::iterator it = std::find(visited_childs.begin(), visited_childs.end(), action_idx);
            int index = std::distance(visited_childs.begin(), it);

            int row = temp_indexs[index][0].item<int>();
            int col = temp_indexs[index][1].item<int>();
            int plane = temp_indexs[index][2].item<int>();

            action_probs = torch::zeros(shape, torch::kFloat32);
            action_probs[row][col][plane] = 1.0f;

            // std::cout << "Temperature choice: " << (float)(get_time_ms() - st) / 1000.0f << " seconds" << std::endl;
            // st = get_time_ms();

            // spGames.at(i)->game->set_state(spGames.at(i)->current_state);
            // spGames.at(i)->game->m_Board->print_board();
            std::string  action = spGames.at(i)->game->decode_action(spGames.at(i)->current_state, action_probs);
            // std::cout << "Action: " << action << std::endl;
            final_state fs = spGames.at(i)->game->get_next_state_and_value(spGames.at(i)->current_state, action, spGames.at(i)->repeated_states);

            spGames.at(i)->repeated_states[fs.board_state.bitboards] += 1;

            if (fs.board_state.halfmove == 0) 
            {
                spGames.at(i)->repeated_states.clear(); // Impossible to repeat states if piece captured or pawn moved
            }

            // std::cout << "Make move: " << (float)(get_time_ms() - st) / 1000.0f << " seconds" << std::endl;
            // st = get_time_ms();

            // Delete the root node

            for (int j = 0; j < spGames.at(i)->pRoot->pChildren.size(); j++)
            {
                delete spGames.at(i)->pRoot->pChildren.at(j);
            }

            if (fs.terminated)
            {
                for (int j = 0; j < spGames.at(i)->memory.size(); j++)
                {
                    float value = (spGames.at(i)->memory.at(j).board_state.side == spGames.at(i)->current_state.side)
                                ? -fs.value
                                : fs.value;

                    torch::Tensor state = torch::zeros({1, 19, 8, 8}, torch::kFloat32); // Initialize the tensor with zeros
                    spGames.at(i)->game->get_encoded_state(state, spGames.at(i)->memory.at(j).board_state);
                    spGames.at(i)->repeated_states.clear();

                    memory.push_back(
                        {
                            
                            state,
                            spGames.at(i)->memory.at(j).action_probs,
                            value
                        }
                    );
                }
                logMessage("Thread: " + std::to_string(thread_id + 1) + 
                    " Game " + std::to_string(i + 1) + 
                    " terminated with " + std::to_string(spGames.at(i)->memory.size()) + " moves" +
                    " Time: " + std::to_string((get_time_ms() - *spg_times.at(i)) / 1000) + " seconds" , log_file);
            
                delete spGames.at(i);
                spGames.erase(spGames.begin() + i);
                delete spg_times.at(i);
                spg_times.erase(spg_times.begin() + i);
            }
            else
            {
                spGames.at(i)->current_state = fs.board_state;
                spGames.at(i)->game->set_state(fs.board_state);
            }
        }
        // logMessage("Thread: " + std::to_string(thread_id + 1) + " Moves: " + std::to_string(count) + " Time: " + std::to_string(((float)(get_time_ms() - st)) / 1000.0f) + " seconds", log_file);
        // std::cout << "Step Time: " << (float)(get_time_ms() - st) / 1000.0f << " seconds" << std::endl;
        // std::cout << "Total Time: " << (float)(get_time_ms() - st_total) / 1000.0f << " seconds" << std::endl;
        // std::cout << "------------------------------------------------" << std::endl;

    }

    return memory;
}

void AlphaZeroMT::learn()
{
    int st = get_time_ms();

    for (int iter = 0; iter < num_iterations; iter++)
    {
        std::vector<std::future<std::vector<sp_memory_item>>> futures;
        std::vector<sp_memory_item> memory;

        // Launch multiple threads to perform self-play
        for (int i = 0; i < num_selfPlay_iterations / (num_threads * num_parallel_games); i++)
        {
            int st = get_time_ms();
            for (int j = 0; j < num_threads; ++j) {
                // logMessage("Self Play Iteration: " + std::to_string(i + 1) + ", Thread: " + std::to_string(j + 1), log_file);
                futures.push_back(std::async(std::launch::async, &AlphaZeroMT::SelfPlay, this, j));
            }
            for (auto& future : futures) {
                try
                {
                    auto sp_memory = future.get(); // Retrieve result once
                    memory.insert(memory.end(), sp_memory.begin(), sp_memory.end());
                }
                catch (const std::exception& e)
                {
                    logMessage("Exception caught during future.get(): " + std::string(e.what()), log_file);
                }
            }
            futures.clear(); // Clear the futures vector before the next iteration
            logMessage("Self Play Iteration: " + std::to_string(i + 1) + " Time: " + std::to_string((float)(get_time_ms() - st) / 1000.0f) + " seconds", log_file);
        }
        update_dichirlet();
        update_temperature();
        update_C();

        logMessage("Memory size: " + std::to_string(memory.size()), log_file);
        
        int st = get_time_ms();
        for (int j = 0; j < num_epochs; j++)
        {
            train(memory);
        }
        logMessage("Training Time: " + std::to_string((float)(get_time_ms() - st) / 1000.0f) + " seconds", log_file);

        for (int i = 0; i < num_threads; i++)
        {
            copy_weights(*m_ResNetChess, *m_ResNetSwarm.at(i));
        }

        logMessage("Weights copied succesfully", log_file);

        save_model();
        
        logMessage("Model saved!!!", log_file);

        // Eval the bot
        st = get_time_ms();
        std::vector<std::future<int>> futuresEval;
        float wins;
        for (int i = 0; i < (num_evals / num_threads); i++)
        {
            for (int j = 0; j < num_threads; ++j) {
                futuresEval.push_back(std::async(std::launch::async, &AlphaZeroMT::AlphaEval, this, j, depth));
            }
            for (auto& future : futuresEval) {
                try
                {
                    auto result = future.get(); // Retrieve result once
                    wins += result;
                }
                catch (const std::exception& e)
                {
                    logMessage("Exception caught during future.get(): " + std::string(e.what()), log_file);
                }
            }
            futuresEval.clear(); // Clear the futures vector before the next iteration
            logMessage("Eval Iteration: " + std::to_string(i + 1) + " Time: " + std::to_string((float)(get_time_ms() - st) / 1000.0f) + " seconds", log_file);
        }
        
        logMessage("Wins %: " + std::to_string(wins / num_evals) + "%, Eval Time: " + std::to_string((float)(get_time_ms() - st) / 1000.0f) + " seconds", log_file);
        if (wins / num_evals > 0.9)
        {
            depth = (depth + 1 > 5) ? 5 : depth + 1;
            logMessage("Depth increased to: " + std::to_string(depth), log_file);
        }
        logMessage("<--------------------------------------------------------->", log_file);
        logMessage("<----------------LEARNING ITERATION----------------------->", log_file);
        logMessage("<--------------------------------------------------------->", log_file);
    }
}

void AlphaZeroMT::train(std::vector<sp_memory_item> memory)
{
    int* pArr = new int[memory.size()];

    // Calculate the size of the array
    int n = memory.size();
    for (int i = 0; i < n; i++)
    {
        pArr[i] = i;
    }
    // Obtain a time-based seed
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

    // Create a random number generator using mt19937
    std::mt19937 rng(seed);

    // Shuffle the array using std::shuffle
    std::shuffle(pArr, pArr + n, rng);

    m_ResNetChess->train();

    float running_loss = 0.0;
    float batch_count = 0.0;
    auto st = get_time_ms();

    if (batch_size > n) batch_size = n;

    for (int i = 0; i < n / batch_size; i++)
    {
        int idx_st = i * batch_size;
        int idx_end = (i + 1) * batch_size;

        if (idx_end > n) idx_end = n;

        unsigned int b_size = abs(idx_end - idx_st);
        
        torch::Tensor encoded_states = torch::zeros({b_size, 19, 8, 8}, torch::kFloat32); // Initialize the tensor with zeros
        torch::Tensor encoded_actions = torch::zeros({b_size, 8, 8, 73}, torch::kFloat32); // Initialize the tensor with zeros
        torch::Tensor values = torch::zeros({b_size, 1}, torch::kFloat32); // Initialize the tensor with zeros

        for (int j = 0; j < b_size; j++)
        {
            encoded_states[j] = memory.at(pArr[idx_st + j]).encoded_state.squeeze(0);
            encoded_actions[j] = memory.at(pArr[idx_st + j]).action_probs.squeeze(0);
            values[j] = memory.at(pArr[idx_st + j]).value;
        }

        encoded_states = encoded_states.to(*m_Device);
        encoded_actions = encoded_actions.to(*m_Device);
        values = values.to(*m_Device);

        auto output = m_ResNetChess->forward(encoded_states);

        auto policy_loss = torch::nn::functional::cross_entropy(output.policy, encoded_actions);
        auto value_loss = torch::nn::functional::mse_loss(output.value, values);
        // std::cout << "Value loss calculated" << std::endl;
        auto loss = policy_loss + value_loss;

        m_Optimizer->zero_grad();
        // std::cout << "Zeroed the gradients" << std::endl;
        loss.backward();
        // std::cout << "Backward pass completed" << std::endl;
        m_Optimizer->step();
        // std::cout << "Optimizer step completed" << std::endl;

        // std::cout << "Loss: " << loss.item<float>() << std::endl;
        // std::cout << "Value loss: " << policy_loss.item<float>() << std::endl;
        running_loss += loss.cpu().item<float>();
        batch_count += 1.0;

    }
    logMessage(" Loss: " + std::to_string(running_loss / batch_count) + " Time: " + std::to_string((float)(get_time_ms() - st) / 1000.0f) + " seconds", log_file);

}

int AlphaZeroMT::AlphaEval(int thread_id, int depth)
{
    std::vector<SPG*> spGames;

    games.at(thread_id)->m_Board->parse_fen(start_position);
    SPG* spg = new SPG(games.at(thread_id));
    spGames.push_back(spg);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    double random_number = dis(gen);

    int alpha_white;
    if (random_number < 0.5)
    {
        alpha_white = 1;
    }
    else
    {
        alpha_white = 0;
    }

    while (true)
    {
        if (spGames.at(0)->game->m_Board->get_side() == alpha_white)
        {
            copy_alpha_board(spGames.at(0)->game->m_Board);

            m_mcts.at(thread_id)->search(&spGames);

            restore_alpha_board(spGames.at(0)->game->m_Board);

            std::vector<int64_t> shape = {8, 8, 73};
            torch::Tensor action_probs = torch::zeros(shape, torch::kFloat32); // Initialize the tensor with zeros

            std::vector<int> visited_childs; 
            for (int j = 0; j < spGames.at(0)->pRoot->pChildren.size(); j++)
            {
                if (spGames.at(0)->pRoot->pChildren.at(j)->visit_count > 0)
                    visited_childs.push_back(j);

                action_probs += 
                    spGames.at(0)->game->get_encoded_action(spGames.at(0)->pRoot->pChildren.at(j)->action, spGames.at(0)->current_state.side) 
                    * spGames.at(0)->pRoot->pChildren.at(j)->visit_count;
            }

            action_probs /= action_probs.sum();

            std::string  move = spGames.at(0)->game->decode_action(spGames.at(0)->current_state, action_probs); 

            games.at(thread_id)->m_Board->make_player_move(move.c_str());
        }
        else
        {
            games.at(thread_id)->m_Board->reset_leaf_nodes();
            float eval = games.at(thread_id)->m_Board->alpha_beta(depth, -1000000, 1000000, true);
            games.at(thread_id)->m_Board->make_bot_move(games.at(thread_id)->m_Board->get_bot_best_move());
        }
        
        state current_state = games.at(thread_id)->get_state();
        spGames.at(0)->current_state = current_state;

        final_state fState = spGames.at(0)->game->get_value_and_terminated(current_state, spGames.at(0)->repeated_states);

        if (fState.terminated)
        {
            if ((current_state.side == alpha_white) && (fState.value == 1.0))
            {
                return 0;
            }
            else if ((current_state.side != alpha_white) && (fState.value == 1.0))
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
    }

    return 0;
}

void AlphaZeroMT::save_model(std::string path)
{
    torch::save(m_ResNetChess, path + "model.pt");
    logMessage( "Model saved in: " + path +"model.pt" , log_file);
}

void AlphaZeroMT::load_model(std::string path)
{
    std::vector<int64_t> shape = {1, 19, 8, 8};
    torch::Tensor encoded_state = torch::rand(shape, torch::kFloat32).to(*m_Device); // Initialize the tensor with zeros
    torch::load(m_ResNetChess, path + "model.pt", *m_Device);
    m_ResNetChess->eval();

    
    auto st = get_time_ms();
    auto output1= m_ResNetChess->forward(encoded_state);
    std::cout << "Before clamp: " << ((get_time_ms() - st) / 1000.0f) << " seconds" << std::endl;
    
    m_ResNetChess->to(*m_Device, torch::kFloat32);

    // clamp_small_weights(*m_ResNetChess, 1e-15);

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

}
