#include "../headers/mctsMT.h"
#include "../headers/game.h"
#include <cmath>
#include <iostream>
#include <thread>
#include <future>


MCTSMT::MCTSMT(int num_searches, float dichirlet_alpha, float dichirlet_epsilon, float C, int num_threads, int num_resblocks, int num_channels, std::string device, std::string pretrained_model_path)
{
    this->num_searches = num_searches;
    this->dichirlet_alpha = dichirlet_alpha;
    this->dichirlet_epsilon = dichirlet_epsilon;
    this->C = C;

    this->num_threads = num_threads;
    this->num_resblocks = num_resblocks;
    this->num_channels = num_channels;

    this->boards_visited.clear();

    if (torch::cuda::is_available() && (device.find("cuda") != device.npos))
    {
        auto dots = device.find(":");
        int device_id = 0;
        if (dots != device.npos)
        {
            device_id = std::stoi(device.substr(dots + 1, device.npos - dots));
        }

        m_Device = std::make_unique<torch::Device>(torch::kCUDA, device_id);
    }
    else
    {
        m_Device = std::make_unique<torch::Device>(torch::kCPU);
    }

    std::cout << pretrained_model_path << std::endl;

    for (int i = 0; i < num_threads; i++)
    {
        auto model = std::make_shared<ResNetChess>(num_resblocks, num_channels, 0.0, *m_Device);

        model->to(*m_Device, torch::kFloat32);

        if (pretrained_model_path != "")
        {
            load_model(model, pretrained_model_path);
        }
        m_ModelSwarm.push_back(model);
    }
    std::cout << "Finished Initialization" << std::endl;
}

MCTSMT::~MCTSMT()
{
}

std::vector<std::tuple<torch::Tensor, float>> MCTSMT::predict(std::vector<SPG *> *spGames)
{

    boards_visited.clear();
    auto st = get_time_ms();
    std::vector<int64_t> shape = {(long)spGames->size(), 19, 8, 8};
    torch::Tensor encoded_state = torch::zeros(shape, torch::kFloat32); // Initialize the tensor with zeros
    for (int i = 0; i < spGames->size(); i++)
    {
        std::vector<int64_t> shape = {(long)spGames->size(), 19, 8, 8};
        torch::Tensor state = torch::zeros({1, 19, 8, 8}, torch::kFloat32); // Initialize the tensor with zeros
        get_encoded_state(state, spGames->at(i)->current_state);
        encoded_state[i] = state.squeeze(0);
    }

    chess_output output_roots;
    {
        torch::NoGradGuard no_grad;

        encoded_state = encoded_state.to(*m_ModelSwarm.at(0)->m_Device);
        output_roots = m_ModelSwarm.at(0)->forward(encoded_state);
        int batch_size = output_roots.policy.size(0);

        output_roots.policy = torch::softmax(output_roots.policy.view({output_roots.policy.size(0), -1}), 1).view({-1, 8, 8, 73});
    }
    // std::cout << "Time to get policy: " << ((get_time_ms() - st) / 1000.0f) << " seconds" << std::endl;
    st = get_time_ms();
    for (int i = 0; i < spGames->size(); i++)
    {
        std::vector<int64_t> shape = {8, 8, 73};
        torch::Tensor spg_policy = output_roots.policy[i].cpu();
        torch::Tensor valid_moves = torch::zeros(shape, torch::kFloat32);

        spGames->at(i)->game->set_state(spGames->at(i)->current_state);

        moves move_list;
        spGames->at(i)->game->m_Board->get_alpha_moves(&move_list);

        get_valid_moves_encoded(valid_moves, spGames->at(i)->current_state, move_list);
        spg_policy *= valid_moves;
        spg_policy /= spg_policy.sum();

        std::vector<double> alpha(move_list.count, dichirlet_alpha);
        add_dirichlet_noise(spg_policy, alpha, dichirlet_epsilon);
        spg_policy /= spg_policy.sum();

        spGames->at(i)->pRoot = new Node(spGames->at(i)->game, nullptr, "", C, 0.0, 1);

        spGames->at(i)->pRoot->expand(spg_policy, valid_moves);
    }

    current_search = 0;
    std::vector<std::future<void>> futures;

    for (int j = 0; j < num_threads; ++j)
    {
        futures.push_back(std::async(std::launch::async, &MCTSMT::search, this, spGames, j));
    }
    for (auto &future : futures)
    {
        try
        {
            future.get(); // Retrieve result once
        }
        catch (const std::exception &e)
        {
            std::cout << ("Exception caught during future.get(): " + std::string(e.what())) << std::endl;
        }
    }

    std::vector<std::tuple<torch::Tensor, float>> results;

    for (int i = 0; i < spGames->size(); i++)
    {

        std::vector<int64_t> shape = {8, 8, 73};
        torch::Tensor action_probs = torch::zeros(shape, torch::kFloat32); // Initialize the tensor with zeros

        std::vector<int> visited_childs;
        std::vector<float> visits_counts;
        std::vector<float> visit_probs;

        float state_value = 0.0f;
        // std::cout << "Visit Counts: [";
        for (int j = 0; j < spGames->at(i)->pRoot->pChildren.size(); j++)
        {
            // std::cout << spGames.at(i)->pRoot->pChildren.at(j)->visit_count << " ";
            if (spGames->at(i)->pRoot->pChildren.at(j)->visit_count > 0)
            {
                visited_childs.push_back(j);
            }

            visit_probs.push_back((float)spGames->at(i)->pRoot->pChildren.at(j)->visit_count / this->get_num_searches());

            state_value += (spGames->at(i)->pRoot->pChildren.at(j)->value_sum / (float)spGames->at(i)->pRoot->pChildren.at(j)->visit_count) * visit_probs[j];
        }

        for (int j = 0; j < spGames->at(i)->pRoot->pChildren.size(); j++)
        {
            action_probs +=
                spGames->at(i)->game->get_encoded_action(spGames->at(i)->pRoot->pChildren.at(j)->action, spGames->at(i)->current_state.side) * visit_probs[j];
        }

        results.push_back(std::make_tuple(action_probs, -state_value));
    }

    return results;
}

void MCTSMT::search(std::vector<SPG *> *spGames, int thread_id)
{
    // std::cout << "Time to expand root: " << ((get_time_ms() - st) / 1000.0f) << " seconds" << std::endl;
    while (current_search < num_searches)
    {
        {
            std::lock_guard<std::mutex> lock(mtxIncreaseSearch);
            current_search++;
        }

        auto st = get_time_ms();

        Node *pCurrentNode = nullptr;
        Node *pNode = spGames->at(0)->pRoot;

        {
            std::lock_guard<std::mutex> lock(mtxSelect);
            while (pNode->is_fully_expanded())
            {
                pNode = pNode->select();
            }

            boards_visited[pNode->node_state.bitboards] += 1;
        }

        final_state fState = spGames->at(0)->game->get_value_and_terminated(pNode->node_state, spGames->at(0)->repeated_states);

        if (fState.terminated)
        {
            {
                std::lock_guard<std::mutex> lock(mtxBackProp);
                pNode->backpropagate(-fState.value);
            }
        }
        else
        {
            pCurrentNode = pNode;
        }

        if (pCurrentNode != nullptr)
        {
            chess_output output_exapandables;
            std::vector<int64_t> exp_shape = {1, 19, 8, 8};
            torch::Tensor encoded_states = torch::zeros(exp_shape, torch::kFloat32); // Initialize the tensor with zeros

            torch::Tensor state = torch::zeros({1, 19, 8, 8}, torch::kFloat32); // Initialize the tensor with zeros
            get_encoded_state(state, pCurrentNode->node_state);
            encoded_states[0] = state.squeeze(0);

            {
                torch::NoGradGuard no_grad;

                encoded_states = encoded_states.to(*m_ModelSwarm.at(thread_id)->m_Device);
                output_exapandables = m_ModelSwarm.at(thread_id)->forward(encoded_states);

                output_exapandables.policy = torch::softmax(output_exapandables.policy.view({output_exapandables.policy.size(0), -1}), 1).view({-1, 8, 8, 73});
            }

            std::vector<int64_t> shape = {8, 8, 73};
            torch::Tensor spg_policy = output_exapandables.policy[0].cpu();
            torch::Tensor valid_moves = torch::zeros(shape, torch::kFloat32);

            spGames->at(0)->game->set_state(pCurrentNode->node_state);

            moves move_list;
            spGames->at(0)->game->m_Board->get_alpha_moves(&move_list);

            get_valid_moves_encoded(valid_moves, pCurrentNode->node_state, move_list);

            spg_policy *= valid_moves;
            spg_policy /= spg_policy.sum();

            {
                std::lock_guard<std::mutex> lock(mtxBackProp);
                if (!pCurrentNode->is_fully_expanded())
                {
                    pCurrentNode->expand(spg_policy, valid_moves);
                }

                st = get_time_ms();
                pCurrentNode->backpropagate(output_exapandables.value[0].cpu().item<float>());
            }
        }
        // std::cout << "Time to expand expandables: " << ((get_time_ms() - st) / 1000.0f) << " seconds" << std::endl;
    }
}

void MCTSMT::load_model(std::shared_ptr<ResNetChess> model, std::string path)
{
    std::vector<int64_t> shape = {1, 19, 8, 8};
    torch::Tensor encoded_state = torch::rand(shape, torch::kFloat32).to(*m_Device); // Initialize the tensor with zeros
    torch::load(model, path + "model.pt", *m_Device);
    model->eval();

    auto st = get_time_ms();
    auto output1 = model->forward(encoded_state);
    std::cout << "Before clamp: " << ((get_time_ms() - st) / 1000.0f) << " seconds" << std::endl;

    model->to(*m_Device, torch::kFloat32);

    // clamp_small_weights(*m_ResNetChess, 1e-15);

    st = get_time_ms();
    auto output2 = model->forward(encoded_state);
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