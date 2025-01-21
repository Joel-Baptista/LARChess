#include "../../src/headers/mcts.h"
#include "../../src/headers/game.h"

#include "../../../BBChessEngine/src/bit_board.h"
#include "../../src/include/json.hpp"
using json = nlohmann::json; 

int main()
{
    std::ifstream config_file("../cfg/config.json");
    if (!config_file.is_open()) {
        std::cerr << "Could not open the config file!" << std::endl;
        return 1;
    }

    // Parse the JSON data
    json config;
    try {
        config_file >> config;
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return 1;
    }

    std::string initial_position = config.value("initial_position", initial_position);
    std::string human_player = config.value("human_player", "w");
    int num_searches = config.value("num_searches", 5);
    float dichirlet_alpha = config.value("dichirlet_alpha", 0.5);
    float dichirlet_epsilon = config.value("dichirlet_epsilon", 0.3);
    float C = config.value("C", 1.41);
    float temperature = config.value("temperature", 1.0);
    int num_resblocks = config.value("num_resblocks", 7);
    int num_channels = config.value("num_channels", 256);
    std::string model_path = config.value("model_path", "");
    std::string device = config.value("device", "cpu");

    std::shared_ptr<torch::Device> m_Device;

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
    
    auto m_ResNetChess = std::make_shared<ResNetChess>(num_resblocks, num_channels, 0.0, *m_Device);
    m_ResNetChess->to(*m_Device, torch::kFloat32);
    std::cout << model_path << std::endl;
    if (model_path != "")
    {
        load_Resnet(model_path, m_ResNetChess, m_Device);
    }
    
    auto mcts = std::make_shared<MCTS>(m_ResNetChess, num_searches, dichirlet_alpha, dichirlet_epsilon, C);

    std::vector<SPG*> spGames;
    auto spg_game = std::make_shared<Game>();
    SPG* spg = new SPG(spg_game);
    spGames.push_back(spg);

    spGames.at(0)->game->m_Board->parse_fen(initial_position.c_str());
    std::string move;

    while (true)
    {
        spGames.at(0)->game->m_Board->print_board();
        state current_state =  spGames.at(0)->game->get_state();
        if (spGames.at(0)->game->m_Board->get_side() == !(human_player == "w" ? 0 : 1))
        {   
            int st = get_time_ms();
            std::cout << "Bot is thinking..." << std::endl;
            auto results = mcts->predict(&spGames, true);
            // std::string move = bot.predict();
            torch::Tensor action_probs = std::get<0>(results.at(0));
            move = spGames.at(0)->game->decode_action(current_state, action_probs);
            std::cout << "Bot move: " << move << " Evaluated at: " << std::get<1>(results.at(0)) << std::endl;
            std::cout << "Bot move made in " << ((get_time_ms() - st) / 1000.0f) << std::endl;
            getchar();
            // spGames.at(0)->game->set_state(current_state);

            // spGames.at(0)->game->m_Board->make_bot_move(spGames.at(0)->game->m_Board->parse_move(move.c_str()));
        }
        else
        {
            moves move_list;
            spGames.at(0)->game->m_Board->get_generate_moves(&move_list);
            
            std::cout << "Moves: [";
            for (int i = 0; i < move_list.count; i++)
            {
                auto move = move_list.moves[i];
                std::cout << spGames.at(0)->game->m_Board->move_to_uci(move) << ", ";

            }
            std::cout << "]" << std::endl;

            bool move_in_list = false;
            while (!move_in_list)
            {
                std::cout << "Enter your move: ";
                std::cin >> move;

                for (const auto& move_item : move_list.moves)
                {
                    int player_move = spGames.at(0)->game->m_Board->parse_move(move.c_str());
                    if (player_move == move_item && player_move != 0)
                    {
                        std::cout << "Player Move: " << player_move << " Move: " << move_item << std::endl;
                        move_in_list = true;
                    }
                }
            }

            // spGames.at(0)->game->m_Board->make_player_move(gui_move.c_str());
        }

        auto fs = spGames.at(0)->game->get_next_state_and_value(spGames.at(0)->current_state, move, spGames.at(0)->repeated_states);

        spGames.at(0)->repeated_states[fs.board_state.bitboards] += 1;

        if (fs.board_state.halfmove == 0) 
        {
            spGames.at(0)->repeated_states.clear(); // Impossible to repeat states if piece captured or pawn moved
        }

        if (fs.terminated)
        {
            std::cout << "Game reached conclusion!" << std::endl;
            spGames.at(0)->game->m_Board->parse_fen(start_position);
            spGames.at(0)->reset();
            break;
        }
        else
        {
            spGames.at(0)->current_state = fs.board_state;
            spGames.at(0)->game->set_state(fs.board_state);
        }

    }
    return 0;
}