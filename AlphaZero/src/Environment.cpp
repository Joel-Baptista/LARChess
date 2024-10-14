#include "Environment.h"

Environment::Environment(
    int id,
    std::shared_ptr<ResNetChess> model,
    std::shared_ptr<MCTS> mcts,
    int num_parallel_games,
    std::shared_ptr<ReplayBuffer> buffer
)
{
    m_Model = model;
    m_Mcts = mcts;
    m_Buffer = buffer;

    for (int i = 0; i < num_parallel_games; i++)
    {
        m_Games.push_back(std::make_shared<Game>());
        m_Games.at(i)->m_Board->parse_fen(start_position);
        SPG* spg = new SPG(m_Games.at(i));
        m_spGames.push_back(spg);
    }
}

Environment::~Environment()
{

}

void Environment::step()
{

    m_Mcts->search(&m_spGames);

    for (int i = 0; i < m_spGames.size(); i++)
    {

        std::vector<int64_t> shape = {8, 8, 73};
        torch::Tensor action_probs = torch::zeros(shape, torch::kFloat32); // Initialize the tensor with zeros
        
        std::vector<int> visited_childs;
        float state_value = 0.0f;
        // std::cout << "Visit Counts: [";
        for (int j = 0; j < m_spGames.at(i)->pRoot->pChildren.size(); j++)
        {
            // std::cout << spGames.at(i)->pRoot->pChildren.at(j)->visit_count << " ";
            if (m_spGames.at(i)->pRoot->pChildren.at(j)->visit_count > 0)
            {
                visited_childs.push_back(j);
            }

            state_value = m_spGames.at(i)->pRoot->pChildren.at(j)->value_sum * 
                (((float)m_spGames.at(i)->pRoot->pChildren.at(j)->visit_count) / m_Mcts->get_num_searches());

            action_probs += 
                m_spGames.at(i)->game->get_encoded_action(m_spGames.at(i)->pRoot->pChildren.at(j)->action, m_spGames.at(i)->current_state.side) 
                * m_spGames.at(i)->pRoot->pChildren.at(j)->visit_count;
        }
        // std::cout << "]" << std::endl;

        // action_probs /= action_probs.sum();
        action_probs = torch::softmax(action_probs.view({action_probs.size(0), -1}), 1).view({-1, 8, 8, 73});

        m_spGames.at(i)->memory.push_back({m_spGames.at(i)->current_state, action_probs, m_spGames.at(i)->current_state.side});

        // std::cout << "Decode action: " << (float)(get_time_ms() - st) / 1000.0f << " seconds" << std::endl;
        // st = get_time_ms();
        

        if (m_spGames.at(i)->current_state.fullmove <= 15)
        {
            torch::Tensor temperature_action_probs = action_probs.pow(1.0);
            temperature_action_probs /= temperature_action_probs.sum();

            auto temp_indexs = torch::nonzero(temperature_action_probs);

            std::vector<double> probabilities(m_spGames.at(i)->pRoot->pChildren.size());

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
        }

        final_state fs;

        std::string  action = m_spGames.at(i)->game->decode_action(m_spGames.at(i)->current_state, action_probs);
        // std::cout << "Action: " << action << std::endl;
        fs = m_spGames.at(i)->game->get_next_state_and_value(m_spGames.at(i)->current_state, action, m_spGames.at(i)->repeated_states);

        m_spGames.at(i)->repeated_states[fs.board_state.bitboards] += 1;

        if (fs.board_state.halfmove == 0) 
        {
            m_spGames.at(i)->repeated_states.clear(); // Impossible to repeat states if piece captured or pawn moved
        }
    
        if (state_value >= 0.05f || !m_spGames.at(i)->early_stop)
        {
            fs.terminated = true;
            fs.value = 1.0f; // Resignation, but we are in the adversaries turn
        }

        delete m_spGames.at(i)->pRoot;

        if (fs.terminated)
        {
            {
                std::lock_guard<std::mutex> lock(m_Buffer->mtxAddBuffer);
                m_Buffer->adding_new_game();
                for (int j = 0; j < m_spGames.at(i)->memory.size(); j++)
                {
                    float value = (m_spGames.at(i)->memory.at(j).board_state.side == m_spGames.at(i)->current_state.side)
                                ? -fs.value
                                : fs.value;

                    torch::Tensor state = torch::zeros({1, 19, 8, 8}, torch::kFloat32); // Initialize the tensor with zeros
                    m_spGames.at(i)->game->get_encoded_state(state, m_spGames.at(i)->memory.at(j).board_state);
                    m_spGames.at(i)->repeated_states.clear();

                    m_Buffer->add(state,  m_spGames.at(i)->memory.at(j).action_probs, torch::tensor(value));
                }
            }

            m_Games.at(i)->m_Board->parse_fen(start_position);
            m_spGames.at(i)->repeated_states.clear();

        }
        else
        {
            m_spGames.at(i)->current_state = fs.board_state;
            m_spGames.at(i)->game->set_state(fs.board_state);
        }


    }

}
