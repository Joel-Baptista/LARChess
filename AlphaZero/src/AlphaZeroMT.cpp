#include "AlphaZeroMT.h"
#include <algorithm> // For std::shuffle
#include <random>    // For std::mt19937
#include <chrono>    // For std::chrono::system_clock



AlphaZeroMT::AlphaZeroMT(
                        int num_searches_init, 
                        int num_searches_max, 
                        float num_searches_ratio,
                        int search_depth,
                        int num_iterations, 
                        int num_selfPlay_iterations, 
                        int num_parallel_games, 
                        int num_epochs, 
                        int max_state_per_game,
                        int swarm_update_freq,
                        int batch_size, 
                        int buffer_size,
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
                        float dropout,
                        int num_resblocks,
                        int num_channels,
                        std::string device,
                        std::string pretrained_model_path,
                        int num_threads
                        )
{



    this->model_path = initLogFiles("../models/alpha");
    log_file = model_path + "/log.txt";

    logMessage("iter,loss", model_path + "/train.csv");

    m_Buffer = std::make_unique<ReplayBuffer>(buffer_size);

    if (torch::cuda::is_available() && (device.find("cuda") != device.npos))
    {
        auto dots = device.find(":");
        int device_id = 0;
        if (dots != device.npos)
        {
            device_id = std::stoi(device.substr(dots + 1, device.npos - dots));
        }

        m_Device = std::make_unique<torch::Device>(torch::kCUDA, device_id);
        log("Using CUDA " + std::to_string(device_id));

        auto stream1 =  c10::cuda::getStreamFromPool();

        for (int i = 0; i < num_threads; ++i) {
            auto stream = c10::cuda::getStreamFromPool();
            cuda_streams.push_back(stream);
        }

    }
    else
    {
        log("Using CPU");
        m_Device = std::make_unique<torch::Device>(torch::kCPU);
    }
    
    m_ResNetChess = std::make_shared<ResNetChess>(num_resblocks, num_channels, dropout, *m_Device);

    std::cout << pretrained_model_path << std::endl;
    if (pretrained_model_path != "")
    {
        load_model(pretrained_model_path);
    }
    
    m_Optimizer = std::make_unique<torch::optim::Adam>(m_ResNetChess->parameters(), torch::optim::AdamOptions(learning_rate).weight_decay(weight_decay));

    for (int i = 0; i < num_threads; i++)
    {
        m_ResNetSwarm.push_back(std::make_shared<ResNetChess>(num_resblocks, num_channels, dropout, *m_Device));
        copy_weights(*m_ResNetChess, *m_ResNetSwarm.at(i));
        m_mcts.push_back(std::make_unique<MCTS>(m_ResNetSwarm.at(i), i, num_searches_init, search_depth, dichirlet_alpha, dichirlet_epsilon, C));
    }


    for (int i = 0; i < num_threads; i++)
    {
        games.push_back(std::make_shared<Game>());
    }

    this->num_searches = num_searches_init;
    this->num_searches_init = num_searches_init;
    this->num_searches_max = num_searches_max;
    this->num_searches_ratio = num_searches_ratio;
    this->search_depth = search_depth;
    this->num_iterations = num_iterations;
    this->num_selfPlay_iterations = num_selfPlay_iterations;
    this->num_parallel_games = num_parallel_games;
    this->num_epochs = num_epochs;
    this->max_state_per_game = max_state_per_game;
    this->swarm_update_freq = swarm_update_freq;
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
    this->dropout = dropout;
    this->num_resblocks = num_resblocks;
    this->num_threads = num_threads;
    this->train_iter = 0;
    this->eval_iter = 0;

    logConfig();
}

AlphaZeroMT::~AlphaZeroMT()
{
}

void AlphaZeroMT::update_dichirlet()
{
    dichirlet_epsilon = std::max(dichirlet_epsilon * dichirlet_epsilon_decay, dichirlet_epsilon_min);

    for (int i = 0; i < num_threads; i++)
        m_mcts.at(i)->set_dichirlet_epsilon(dichirlet_epsilon);

    log("Dichirlet epsilon: " + std::to_string(dichirlet_epsilon) + " Dichirlet alpha: " + std::to_string(dichirlet_alpha));   
}

void AlphaZeroMT::update_temperature()
{
    temperature = std::max(temperature * temperature_decay, temperature_min);

    log("Temperature: " + std::to_string(temperature));   
}

void AlphaZeroMT::update_C()
{
    C = std::max(C * C_decay, C_min);

    for (int i = 0; i < num_threads; i++)
        m_mcts.at(i)->set_C(C);

    log("C: " + std::to_string(C));   
}

void AlphaZeroMT::update_num_searches()
{
    num_searches = std::min((int)(num_searches * num_searches_ratio), num_searches_max);

    for (int i = 0; i < num_threads; i++)
        m_mcts.at(i)->set_num_searches(num_searches);

    log("num_searches: " + std::to_string(num_searches));   
}

void AlphaZeroMT::SelfPlay(int thread_id)
{
    std::vector<SPG*> spGames;
    std::vector<int*> spg_times;

    m_ResNetSwarm.at(thread_id)->eval();

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

        if (m_Device->is_cuda())
        {
            m_mcts.at(thread_id)->search(&spGames, cuda_streams);
        }
        else
        {
            m_mcts.at(thread_id)->search(&spGames);
        }

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

            // for (int j = 0; j < spGames.at(i)->pRoot->pChildren.size(); j++)
            // {
            //     delete spGames.at(i)->pRoot->pChildren.at(j);
            // }
            delete spGames.at(i)->pRoot;

            if (fs.terminated)
            {
                {
                    std::lock_guard<std::mutex> lock(mtxBuffer);
                    m_Buffer->adding_new_game();
                    for (int j = 0; j < spGames.at(i)->memory.size(); j++)
                    {
                        float value = (spGames.at(i)->memory.at(j).board_state.side == spGames.at(i)->current_state.side)
                                    ? -fs.value
                                    : fs.value;

                        torch::Tensor state = torch::zeros({1, 19, 8, 8}, torch::kFloat32); // Initialize the tensor with zeros
                        spGames.at(i)->game->get_encoded_state(state, spGames.at(i)->memory.at(j).board_state);
                        spGames.at(i)->repeated_states.clear();

                        m_Buffer->add(state,  spGames.at(i)->memory.at(j).action_probs, torch::tensor(value));
                    }
                }

                // std::cout << "Memory added to the buffer" << std::endl;
                // log("Thread: " + std::to_string(thread_id + 1) + 
                //     " Game " + std::to_string(i + 1) + 
                //     " terminated with " + std::to_string(spGames.at(i)->memory.size()) + " moves" +
                //     " Time: " + std::to_string((get_time_ms() - *spg_times.at(i)) / 1000) + " seconds" );
            
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
        // log("Thread: " + std::to_string(thread_id + 1) + " Moves: " + std::to_string(count) + " Time: " + std::to_string(((float)(get_time_ms() - st)) / 1000.0f) + " seconds");
        // std::cout << "Num of games: " << spGames.size() << std::endl;
        // std::cout << "Step Time: " << (float)(get_time_ms() - st) / 1000.0f << " seconds" << std::endl;
        // std::cout << "Total Time: " << (float)(get_time_ms() - st_total) / 1000.0f << " seconds" << std::endl;
        // std::cout << "------------------------------------------------" << std::endl;

    }

}

void AlphaZeroMT::learn()
{
    int st = get_time_ms();

    while (train_iter < num_iterations)
    {
        std::vector<std::future<void>> futures;

        // Launch multiple threads to perform self-play

        int st = get_time_ms();
        for (int j = 0; j < num_threads; ++j) {
            // log("Self Play Iteration: " + std::to_string(i + 1) + ", Thread: " + std::to_string(j + 1));
            futures.push_back(std::async(std::launch::async, &AlphaZeroMT::SelfPlay, this, j));
        }
        for (auto& future : futures) {
            try
            {
                future.get(); // Retrieve result once
            }
            catch (const std::exception& e)
            {
                log("Exception caught during future.get(): " + std::string(e.what()));
            }
        }
        futures.clear(); // Clear the futures vector before the next iteration
        log("Self Play Iteration Time: " + std::to_string((float)(get_time_ms() - st) / 1000.0f) + " seconds");
        

        if (m_Buffer->size() < batch_size) // Wait until we have enough data and is possible to sample less than 30 positions 
        {
            log("Buffer size: " + std::to_string(m_Buffer->size()) + " is less than batch size: " + std::to_string(batch_size));
            continue;
        }

        if ( (m_Buffer->get_current_game_id() < (batch_size / max_state_per_game)))
        {
            log("Current Game Id: " + std::to_string(m_Buffer->get_current_game_id()) + " is less than batch size: " + std::to_string(batch_size / max_state_per_game));
            continue;
        }

        st = get_time_ms();

        train();

        log("Training Time: " + std::to_string((float)(get_time_ms() - st) / 1000.0f) + " seconds");

        save_model(model_path);
        
        log("Model saved!!!");

        // Eval the bot
        std::vector<std::future<int>> futuresEval;
        evalResults results;
        for (int i = 0; i < (num_evals / num_threads); i++)
        {
            st = get_time_ms();
            for (int j = 0; j < num_threads; ++j) {
                futuresEval.push_back(std::async(std::launch::async, &AlphaZeroMT::AlphaEval, this, j, depth));
            }
            for (auto& future : futuresEval) {
                try
                {
                    auto result = future.get(); // Retrieve result once
                    if (result == 1)
                        results.win_count++;
                    else if (result == 0)
                        results.draw_count++;
                    else
                        results.loss_count++;                    
                }
                catch (const std::exception& e)
                {
                    log("Exception caught during future.get(): " + std::string(e.what()));
                }
            }
            futuresEval.clear(); // Clear the futures vector before the next iteration
            log("Eval Iteration: " + std::to_string(i + 1) + " Time: " + std::to_string((float)(get_time_ms() - st) / 1000.0f) + " seconds");
        }
        
        log("Wins %: " + std::to_string(results.win_count / (float)num_evals) + "%, " +  
            "Loss %: " + std::to_string(results.loss_count / (float)num_evals) + "%, " +
            "Draw %: " + std::to_string(results.draw_count / (float)num_evals) +  
            "%, Eval Time: " + std::to_string((float)(get_time_ms() - st) / 1000.0f) + " seconds");

        logEval(std::to_string(eval_iter) + "," + 
        std::to_string(results.win_count / (float)num_evals) + "," + 
        std::to_string(results.loss_count / (float)num_evals) + "," + 
        std::to_string(results.draw_count / (float)num_evals));
        eval_iter++;

        if (results.win_count / num_evals > 0.5)
        {
            depth = (depth + 1 > 5) ? 5 : depth + 1;
            log("Depth increased to: " + std::to_string(depth));
        }
        update_dichirlet();
        update_temperature();
        update_C();
        update_num_searches();
        log("<--------------------------------------------------------->");
        log("<----------------LEARNING ITERATION----------------------->");
        log("<--------------------------------------------------------->");
    }
}

void AlphaZeroMT::train()
{

    m_ResNetChess->train();

    float running_loss = 0.0;
    float batch_count = 0.0;
    auto st = get_time_ms();

    for (int i = 0; i < num_epochs; i++)
    {
        
        auto samples = m_Buffer->sample(batch_size, max_state_per_game);

        torch::Tensor encoded_states = samples.states;
        torch::Tensor encoded_actions = samples.action_probs;
        torch::Tensor values = samples.values;
        
        encoded_states = encoded_states.to(*m_Device);
        encoded_actions = encoded_actions.to(*m_Device);
        values = values.to(*m_Device);

        auto output = m_ResNetChess->forward(encoded_states);

        auto policy_loss = torch::nn::functional::cross_entropy(output.policy, encoded_actions);
        auto value_loss = torch::nn::functional::mse_loss(output.value.squeeze(1), values);
        // std::cout << "Value loss calculated" << std::endl;
        auto loss = policy_loss + value_loss;

        m_Optimizer->zero_grad();
        // std::cout << "Zeroed the gradients" << std::endl;
        loss.backward();
        // std::cout << "Backward pass completed" << std::endl;
        m_Optimizer->step();
        
        log(" Loss: " + std::to_string(loss.cpu().item<float>()) + " Time: " + std::to_string((float)(get_time_ms() - st) / 1000.0f) + " seconds");
        logTrain(std::to_string(train_iter) + "," + std::to_string(loss.cpu().item<float>()));
        train_iter++;

        if  (train_iter % swarm_update_freq == 0)
        {
            // Hard copy the weights to the swarm (for now)
            for (int i = 0; i < num_threads; i++)
            {
                copy_weights(*m_ResNetChess, *m_ResNetSwarm.at(i));
            }

            log("Data Aquisition Nets Updated");
        }

    }




}

int AlphaZeroMT::AlphaEval(int thread_id, int depth)
{
    std::vector<SPG*> spGames;
    evalResults results;

    m_ResNetSwarm.at(thread_id)->eval();

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

            if (m_Device->is_cuda())
            {
                m_mcts.at(thread_id)->search(&spGames, cuda_streams);
            }
            else
            {
                m_mcts.at(thread_id)->search(&spGames);
            }

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

            delete spGames.at(0)->pRoot;

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
            int res;
            if ((current_state.side == alpha_white) && (fState.value == 1.0))
            {
                res = -1;
            }
            else if ((current_state.side != alpha_white) && (fState.value == 1.0))
            {
                res = 1;
            }
            else
            {
                res = 0;
            }
            delete spGames.at(0);
            spGames.erase(spGames.begin());
            return res;
        }
    }

    return 0;
}

void AlphaZeroMT::save_model(std::string path)
{
    if (path == "")
    {
        torch::save(m_ResNetChess, "model.pt");
        log( "Model saved in: model.pt" );
    }
    else
    {
        torch::save(m_ResNetChess, path + "/model.pt");
        log( "Model saved in: " + path + "/model.pt" );
    }
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

void AlphaZeroMT::log(std::string message)
{
    logMessage( "[" + getCurrentTimestamp() + "] " + message, model_path + "/log.txt");
    std::cout << "[" << getCurrentTimestamp() << "] " << message << std::endl;
}

void AlphaZeroMT::logTrain(std::string message)
{
    logMessage(message, model_path + "/train.csv");
}

void AlphaZeroMT::logEval(std::string message)
{
    logMessage(message, model_path + "/eval.csv");
}

void AlphaZeroMT::logConfig()
{
    logMessage("{", model_path + "/config.json");
    logMessage("    \"num_searches_init\": \"" + std::to_string(num_searches_init) + "\",", model_path + "/config.json");
    logMessage("    \"num_searches_max\": \"" + std::to_string(num_searches_max) + "\",", model_path + "/config.json");
    logMessage("    \"num_searches_ratio\": \"" + std::to_string(num_searches_ratio) + "\",", model_path + "/config.json");
    logMessage("    \"search_depth\": \"" + std::to_string(search_depth) + "\",", model_path + "/config.json");
    logMessage("    \"num_iterations\": \"" + std::to_string(num_iterations) + "\",", model_path + "/config.json");
    logMessage("    \"num_selfPlay_iterations\": \"" + std::to_string(num_selfPlay_iterations) + "\",", model_path + "/config.json");
    logMessage("    \"num_parallel_games\": \"" + std::to_string(num_parallel_games) + "\",", model_path + "/config.json");
    logMessage("    \"num_epochs\": \"" + std::to_string(num_epochs) + "\",", model_path + "/config.json");
    logMessage("    \"max_state_per_game\": \"" + std::to_string(max_state_per_game) + "\",", model_path + "/config.json");
    logMessage("    \"swarm_update_freq\": \"" + std::to_string(swarm_update_freq) + "\",", model_path + "/config.json");
    logMessage("    \"batch_size\": \"" + std::to_string(batch_size) + "\",", model_path + "/config.json");
    logMessage("    \"buffer_size,\": \"" + std::to_string(buffer_size) + "\",", model_path + "/config.json");
    logMessage("    \"temperature\": \"" + std::to_string(temperature) + "\",", model_path + "/config.json");
    logMessage("    \"temperature_min\": \"" + std::to_string(temperature_min) + "\",", model_path + "/config.json");
    logMessage("    \"learning_rate\": \"" + std::to_string(learning_rate) + "\",", model_path + "/config.json");
    logMessage("    \"dichirlet_alpha\": \"" + std::to_string(dichirlet_alpha) + "\",", model_path + "/config.json");
    logMessage("    \"dichirlet_epsilon\": \"" + std::to_string(dichirlet_epsilon) + "\",", model_path + "/config.json");
    logMessage("    \"dichirlet_epsilon_decay\": \"" + std::to_string(dichirlet_epsilon_decay) + "\",", model_path + "/config.json");
    logMessage("    \"dichirlet_epsilon_min\": \"" + std::to_string(dichirlet_epsilon_min) + "\",", model_path + "/config.json");
    logMessage("    \"C\": \"" + std::to_string(C) + "\",", model_path + "/config.json");
    logMessage("    \"C_decay\": \"" + std::to_string(C_decay) + "\",", model_path + "/config.json");
    logMessage("    \"C_min\": \"" + std::to_string(C_min) + "\",", model_path + "/config.json");
    logMessage("    \"num_evals\": \"" + std::to_string(num_evals) + "\",", model_path + "/config.json");
    logMessage("    \"depth\": \"" + std::to_string(depth) + "\",", model_path + "/config.json");
    logMessage("    \"weight_decay\": \"" + std::to_string(weight_decay) + "\",", model_path + "/config.json");
    logMessage("    \"dropout\": \"" + std::to_string(dropout) + "\",", model_path + "/config.json");
    logMessage("    \"num_resblocks\": \"" + std::to_string(num_resblocks) + "\",", model_path + "/config.json");
    logMessage("    \"num_threads\": \"" + std::to_string(num_threads) + "\"", model_path + "/config.json");
    logMessage("}", model_path + "/config.json");
}