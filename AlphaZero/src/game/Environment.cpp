#include "Environment.h"

Environment::Environment(
    int id,
    std::shared_ptr<ResNetChess> model,
    std::shared_ptr<MCTS> mcts,
    int num_parallel_games,
    std::shared_ptr<ReplayBuffer> buffer,
    float early_stopping,
    float early_stopping_value
)
{
    m_Model = model;
    m_Mcts = mcts;
    m_Buffer = buffer;
    m_early_stopping = early_stopping;
    m_early_stopping_value = early_stopping_value;

    for (int i = 0; i < num_parallel_games; i++)
    {
        auto game = std::make_shared<Game>();
        game->m_Board->parse_fen(start_position);
        SPG* spg = new SPG(game, m_early_stopping);
        m_spGames.push_back(spg);
    }
}

Environment::~Environment()
{

}

void Environment::step()
{
    auto results = m_Mcts->predict(&m_spGames);

    for (int i = 0; i < m_spGames.size(); i++)
    {
        
        torch::Tensor action_probs = std::get<0>(results.at(i));
        float state_value = std::get<1>(results.at(i));

        m_spGames.at(i)->memory.push_back({m_spGames.at(i)->current_state, action_probs, m_spGames.at(i)->current_state.side});

        if (m_spGames.at(i)->current_state.fullmove <= 15)
        {
            torch::Tensor temperature_action_probs = action_probs.pow(1.0);
            temperature_action_probs /= temperature_action_probs.sum();

            auto temp_indexs = torch::nonzero(temperature_action_probs);
            
            torch::Tensor sampled_index = torch::multinomial(temperature_action_probs.view({-1}), 1, true);
            
            action_probs = torch::zeros({8 * 8 * 73}, torch::kFloat32);
            action_probs[sampled_index.item<int>()] = 1.0f;
        }

        final_state fs;
        action_probs = action_probs.view({8, 8, 73});

        std::string  action = m_spGames.at(i)->game->decode_action(m_spGames.at(i)->current_state, action_probs);
        // std::cout << "Action: " << action << std::endl;
        fs = m_spGames.at(i)->game->get_next_state_and_value(m_spGames.at(i)->current_state, action, m_spGames.at(i)->repeated_states);

        m_spGames.at(i)->repeated_states[fs.board_state.bitboards] += 1;

        if (fs.board_state.halfmove == 0) 
        {
            m_spGames.at(i)->repeated_states.clear(); // Impossible to repeat states if piece captured or pawn moved
        }

    
        if (state_value <= m_early_stopping_value && m_spGames.at(i)->early_stop)
        {
            std::cout << "State value: " << state_value << " Early Stoping: " << m_spGames.at(i)->early_stop << std::endl; 
            fs.terminated = true;
            fs.value = -1.0f; // Resignation
        }

        delete m_spGames.at(i)->pRoot;

        if (fs.terminated)
        {
            {
                std::lock_guard<std::mutex> lock(m_Buffer->mtxAddBuffer);
                m_Buffer->adding_new_game();
                int len = 0;
                for (int j = 0; j < m_spGames.at(i)->memory.size(); j++)
                {
                    len++;
                    float value = (m_spGames.at(i)->memory.at(j).board_state.side == m_spGames.at(i)->current_state.side)
                                ? fs.value
                                : -fs.value;

                    torch::Tensor state = torch::zeros({1, 19, 8, 8}, torch::kFloat32); // Initialize the tensor with zeros
                    m_spGames.at(i)->game->get_encoded_state(state, m_spGames.at(i)->memory.at(j).board_state);
                    m_spGames.at(i)->repeated_states.clear();

                    m_Buffer->add(state,  m_spGames.at(i)->memory.at(j).action_probs, torch::tensor(value));
                }

                m_Buffer->add_stats(torch::tensor(fs.value).abs(), torch::tensor(len));
            }

            m_spGames.at(i)->game->m_Board->parse_fen(start_position);
            m_spGames.at(i)->reset();

        }
        else
        {
            m_spGames.at(i)->current_state = fs.board_state;
            m_spGames.at(i)->game->set_state(fs.board_state);
        }


    }

}
