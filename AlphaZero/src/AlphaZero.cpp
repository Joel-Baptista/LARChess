#include "AlphaZero.h"


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
    m_ResNetChess = std::make_unique<ResNetChess>(num_resblocks, 256, torch::kCPU);

    m_Optimizer = std::make_unique<torch::optim::Adam>(m_ResNetChess->parameters(), torch::optim::AdamOptions(learning_rate).weight_decay(weight_decay));
    m_Device = std::make_unique<torch::Device>(torch::kCPU);

    m_mcts = std::make_unique<MCTS>(num_searches, dichirlet_alpha, dichirlet_epsilon, C);

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
}


void AlphaZero::SelfPlay()
{
    std::vector<SPG*> spGames;
    BitBoard board;
    board.parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    for (int i = 0; i < num_parallel_games; i++)
    {
        SPG* spg = new SPG(game);
        spGames.push_back(spg);
    }

    int count = 0;
    while (spGames.size() > 0)
    {
        count++;
        std::cout << "Self Play Iteration: " << count << std::endl;

        m_mcts->search(&spGames);

        for (int i = spGames.size() - 1; i >= 0; i--)
        {

            std::array<std::size_t, 3> shape = {8, 8, 73}; 
            xt::xtensor<float, 3> valid_moves(shape);
            valid_moves.fill(2.0f);
            
            xt::xtensor<float, 3> action_probs(shape);
            action_probs.fill(1.0f);

            for (int j = 0; j < spGames.at(i)->pRoot->pChildren.size(); j++)
            {
                std::array<std::size_t, 3> shape = {8, 8, 73}; 
                xt::xtensor<float, 3> encoded_actions(shape);
                action_probs *= encoded_actions * spGames.at(i)->pRoot->pChildren.at(j)->visit_count;
            }

            action_probs /= xt::sum(action_probs)();

            spGames.at(i)->memory.push_back({spGames.at(i)->current_state, action_probs, 1});

            // TODO Implement the temperature scaling


            
        }
    }

    
}

