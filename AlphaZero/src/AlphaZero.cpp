#include "AlphaZero.h"
#include <algorithm> // For std::shuffle
#include <random>    // For std::mt19937
#include <chrono>    // For std::chrono::system_clock


AlphaZero::AlphaZero(Game* game,
                     int num_searches, 
                     int num_iterations, 
                     int num_selfPlay_iterations, 
                     int num_parallel_games, 
                     int num_epochs, 
                     int batch_size, 
                     float temperature, 
                     float learning_rate, 
                     float dichirlet_alpha, 
                     float dichirlet_epsilon, 
                     float C,
                     float weight_decay,
                     int num_resblocks)
{
    m_ResNetChess = new ResNetChess(num_resblocks, 256, torch::kCPU);

    m_Optimizer = std::make_unique<torch::optim::Adam>(m_ResNetChess->parameters(), torch::optim::AdamOptions(learning_rate).weight_decay(weight_decay));
    m_Device = std::make_unique<torch::Device>(torch::kCPU);

    m_mcts = std::make_unique<MCTS>(m_ResNetChess, num_searches, dichirlet_alpha, dichirlet_epsilon, C);

    this->game = game;
    this->num_searches = num_searches;
    this->num_iterations = num_iterations;
    this->num_selfPlay_iterations = num_selfPlay_iterations;
    this->num_parallel_games = num_parallel_games;
    this->num_epochs = num_epochs;
    this->batch_size = batch_size;
    this->temperature = temperature;
    this->learning_rate = learning_rate;
    this->dichirlet_alpha = dichirlet_alpha;
    this->dichirlet_epsilon = dichirlet_epsilon;
    this->C = C;
    this->weight_decay = weight_decay;
    this->num_resblocks = num_resblocks;
}

AlphaZero::~AlphaZero()
{
    delete m_ResNetChess;
}


std::vector<sp_memory_item> AlphaZero::SelfPlay()
{
    std::vector<sp_memory_item> memory;
    std::vector<SPG*> spGames;
    game->m_Board->parse_fen(start_position);

    for (int i = 0; i < num_parallel_games; i++)
    {
        SPG* spg = new SPG(game);
        spGames.push_back(spg);
    }

    int count = 0;
    while (spGames.size() > 0)
    {

        count++;
        std::cout << "Game Iteration: " << count << std::endl;

        m_mcts->search(&spGames);

        for (int i = spGames.size() - 1; i >= 0; i--)
        {
            std::array<std::size_t, 3> shape = {8, 8, 73}; 

            xt::xtensor<float, 3> action_probs(shape);
            action_probs.fill(0.0f);
            
            for (int j = 0; j < spGames.at(i)->pRoot->pChildren.size(); j++)
            {
                action_probs += 
                    spGames.at(i)->game->get_encoded_action(spGames.at(i)->pRoot->pChildren.at(j)->action, spGames.at(i)->current_state.side) 
                    * spGames.at(i)->pRoot->pChildren.at(j)->visit_count;
            }

            action_probs /= xt::sum(action_probs)();

            spGames.at(i)->memory.push_back({spGames.at(i)->current_state, action_probs, spGames.at(i)->current_state.side});

            // TODO Implement the temperature scaling

            std::string  action = spGames.at(i)->game->decode_action(spGames.at(i)->current_state, action_probs);

            final_state fs = spGames.at(i)->game->get_next_state_and_value(spGames.at(i)->current_state, action);
        

            if (fs.terminated)
            {
                std::cout << "GAME TERMINATED" << std::endl;
                for (int j = 0; j < spGames.at(i)->memory.size(); j++)
                {
                    int value = (spGames.at(i)->memory.at(j).board_state.side == spGames.at(i)->current_state.side)
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
                std::cout << "Deleting game" << std::endl;
                delete spGames.at(i);
                spGames.erase(spGames.begin() + i);
                std::cout << "Deleted game" << std::endl;
            }
            else
            {
                spGames.at(i)->current_state = fs.board_state;
                spGames.at(i)->game->set_state(fs.board_state);
            }
        }
    }

    return memory;
}

void AlphaZero::learn()
{

    int st = get_time_ms();

    for (int iter = 0; iter < num_iterations; iter++)
    {
        std::vector<sp_memory_item> memory;
        for (int i = 0; i < num_selfPlay_iterations / num_parallel_games ; i++)
        {
            std::cout << "Self Play Iteration: " << i + 1 << std::endl;
            auto sp_memory = SelfPlay();
            memory.insert(memory.end(), sp_memory.begin(), sp_memory.end());
        }

        for (int j = 0; j < num_epochs; j++)
        {
            train(memory);
        }
    }
}

void AlphaZero::train(std::vector<sp_memory_item> memory)
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

    for (int i = 0; i < n / batch_size; i++)
    {

        int idx_st = i * batch_size;
        int idx_end = (i + 1) * batch_size;

        if (idx_end > n) idx_end = n;

        unsigned int b_size = abs(idx_end - idx_st);
        
        std::array<std::size_t, 4> state_shape = {b_size, 19, 8, 8}; 
        std::array<std::size_t, 4> action_shape = {b_size, 8, 8, 73}; 
        std::array<std::size_t, 2> value_shape = {b_size, 1}; 
        xt::xtensor<float, 4> states(state_shape);
        xt::xtensor<float, 4> action_targets(action_shape);
        xt::xtensor<float, 2> values_targets(value_shape);

        for (int j = idx_st; j < idx_end - 1; j++)
        {
            xt::view(states, idx_st + j, xt::all(), xt::all(), xt::all()) = memory.at(pArr[j]).encoded_state;
            xt::view(action_targets, idx_st + j, xt::all(), xt::all(), xt::all()) = memory.at(pArr[j]).action_probs;
            xt::view(values_targets, idx_st + j, xt::all()) = memory.at(pArr[j]).value;
        }

        std::cout << "Training Iteration: " << i + 1 << std::endl;

        auto torch_states = xtensor_to_torch(states);
        auto torch_action_targets = xtensor_to_torch(action_targets);
        auto torch_values_targets = xtensor_to_torch(values_targets);
        
        // std::cout << "State size: " << states << std::endl; 
        // auto output = m_ResNetChess->forward(torch_states);
        // std::cout << "Forward pass completed" << std::endl;

        // auto policy_loss = torch::nn::functional::cross_entropy(output.policy, torch_action_targets);
        // std::cout << "Policy loss calculated" << std::endl;
        // std::cout << "Value size: " << output.value.sizes() << std::endl;
        // std::cout << "Value size: " << xtensor_to_torch(values_targets).sizes() << std::endl;
        // auto value_loss = torch::nn::functional::mse_loss(output.value, torch_values_targets);
        // std::cout << "Value loss calculated" << std::endl;
        // auto loss = policy_loss + value_loss;

        // m_Optimizer->zero_grad();
        // std::cout << "Zeroed the gradients" << std::endl;
        // loss.backward();
        // std::cout << "Backward pass completed" << std::endl;
        // m_Optimizer->step();
        // std::cout << "Optimizer step completed" << std::endl;

        // std::cout << "Loss: " << loss.item<float>() << std::endl;

    }

}