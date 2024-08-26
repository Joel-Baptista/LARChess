#include "alphabot.h"

AlphaBot::AlphaBot(std::shared_ptr<Game> game, 
                   int num_searches, 
                   float C, 
                   float dichirlet_alpha, 
                   float dichirlet_epsilon, 
                   int num_resblocks,
                   int num_channels,
                   std::string device)
{
    m_Game = game;
    m_NumSearches = num_searches;
    m_C = C;
    m_Device = std::make_shared<torch::Device>(torch::kCPU);
    if (device == "cuda")
    {
        m_Device = std::make_shared<torch::Device>(torch::kCUDA);
    }

    m_ResNetChess = std::make_shared<ResNetChess>(num_resblocks, num_channels, *m_Device);
    m_mcts = std::make_unique<MCTS>(m_ResNetChess, num_searches, dichirlet_alpha, dichirlet_epsilon, C);
}

AlphaBot::~AlphaBot()
{
}

void AlphaBot::load_model(std::string path)
{
    torch::load(m_ResNetChess, path + "model.pt", *m_Device);
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
    for (int j = 0; j < spGames.at(0)->pRoot->pChildren.size(); j++)
    {
        action_probs += 
            spGames.at(0)->game->get_encoded_action(spGames.at(0)->pRoot->pChildren.at(j)->action, spGames.at(0)->current_state.side) 
            * spGames.at(0)->pRoot->pChildren.at(j)->visit_count;
    }
    action_probs /= action_probs.sum();
    std::string  move = spGames.at(0)->game->decode_action(spGames.at(0)->current_state, action_probs);
    std::cout << "Time to get action probs: " << ((get_time_ms() - st) / 1000.0f) << " seconds" << std::endl;
    return move;
}

void AlphaBot::make_bot_move(std::string move)
{
    m_Game->m_Board->make_move(m_Game->m_Board->parse_move(move.c_str()));
}