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
                        float weight_decay,
                        int num_resblocks,
                        int num_channels,
                        std::string device,
                        int num_threads
                        )
{

    log_file = "log.txt";
    initLogFile(log_file);

    if (device == "cuda")
    {
        m_Device = std::make_unique<torch::Device>(torch::kCUDA);
        logMessage("Using CUDA", log_file);
    }
    else
    {
        logMessage("Using CPU", log_file);
        m_Device = std::make_unique<torch::Device>(torch::kCPU);
    }

    m_ResNetChess = std::make_shared<ResNetChess>(num_resblocks, num_channels, *m_Device);
    
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

        count++;

        m_mcts.at(thread_id)->search(&spGames);

        for (int i = spGames.size() - 1; i >= 0; i--)
        {

            std::vector<int64_t> shape = {8, 8, 73};
            torch::Tensor action_probs = torch::zeros(shape, torch::kFloat32); // Initialize the tensor with zeros
            
            for (int j = 0; j < spGames.at(i)->pRoot->pChildren.size(); j++)
            {
                action_probs += 
                    spGames.at(i)->game->get_encoded_action(spGames.at(i)->pRoot->pChildren.at(j)->action, spGames.at(i)->current_state.side) 
                    * spGames.at(i)->pRoot->pChildren.at(j)->visit_count;
            }

            action_probs /= action_probs.sum();

            spGames.at(i)->memory.push_back({spGames.at(i)->current_state, action_probs, spGames.at(i)->current_state.side});
            
            torch::Tensor temperature_action_probs = action_probs.pow(1.0 / temperature);
            temperature_action_probs /= temperature_action_probs.sum();

            std::vector<double> probabilities(spGames.at(i)->pRoot->pChildren.size());
            int move_count = 0;
            for (int i = 0; i < 8; i++)
            {
                for (int j = 0; j < 8; j++)
                {
                    for (int k = 0; k < 73; k++)
                    {
                        if (temperature_action_probs[i][j][k].item<double>() > 0.0f)
                        {
                            probabilities[move_count] = temperature_action_probs[i][j][k].item<double>();
                            move_count++;
                        }
                    }
                }
            }

            std::random_device rd;
            std::mt19937 gen(rd());
            std::discrete_distribution<> dist(probabilities.begin(), probabilities.end());

            int action_idx = dist(gen);

            int action_count = 0;
            for (int i = 0; i < 8; i++)
            {
                for (int j = 0; j < 8; j++)
                {
                    for (int k = 0; k < 73; k++)
                    {
                        if (temperature_action_probs[i][j][k].item<double>() > 0.0f)
                        {
                            if (action_count == action_idx)
                            {
                                action_probs = torch::zeros(shape, torch::kFloat32);
                                action_probs[i][j][k] = 1.0f;
                                break;
                            }
                            action_count++;
                        }
                    }
                }
            }

            // spGames.at(i)->game->set_state(spGames.at(i)->current_state);
            // spGames.at(i)->game->m_Board->print_board();
            std::string  action = spGames.at(i)->game->decode_action(spGames.at(i)->current_state, action_probs);

            final_state fs = spGames.at(i)->game->get_next_state_and_value(spGames.at(i)->current_state, action, spGames.at(i)->repeated_states);

            spGames.at(i)->repeated_states[fs.board_state.bitboards] += 1;
            if (fs.board_state.halfmove == 0) 
            {
                spGames.at(i)->repeated_states.clear(); // Impossible to repeat states if piece captured or pawn moved
            }

            if (fs.terminated)
            {
                for (int j = 0; j < spGames.at(i)->memory.size(); j++)
                {
                    float value = (spGames.at(i)->memory.at(j).board_state.side == spGames.at(i)->current_state.side)
                                ? fs.value
                                : -fs.value;

                    memory.push_back(
                        {
                            
                            spGames.at(i)->game->get_encoded_state(spGames.at(i)->memory.at(j).board_state),
                            spGames.at(i)->memory.at(j).action_probs,
                            value
                        }
                    );
                }
                // logMessage("Thread: " + std::to_string(thread_id + 1) + 
                //     " Game " + std::to_string(i + 1) + 
                //     " terminated with " + std::to_string(spGames.at(i)->memory.size()) + " moves" +
                //     " Time: " + std::to_string((get_time_ms() - *spg_times.at(i)) / 1000) + " seconds" , log_file);
            
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
        // logMessage("Thread: " + std::to_string(thread_id + 1) + " Iteration: " + std::to_string(count) + " Time: " + std::to_string(((float)(get_time_ms() - st)) / 1000.0f) + " seconds", log_file);
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

        std::cout << "Memory size: " << memory.size() << std::endl;
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

        save_model();
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

void AlphaZeroMT::save_model(std::string path)
{
    torch::save(m_ResNetChess, path + "model.pt");
}

void AlphaZeroMT::load_model(std::string path)
{
    torch::load(m_ResNetChess, path + "model.pt");
}
