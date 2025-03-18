#include "../../src/headers/mcts.h"
#include "../../src/headers/game.h"

#include "../../../BBChessEngine/src/bit_board.h"
#include "../../src/include/json.hpp"
#include <fstream>
#include <omp.h>

using json = nlohmann::json; 



// Function to write chess positions to a CSV file
void saveToCSV(const std::vector<std::pair<std::string, std::string>>& gameData, const std::string& filename) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    // Write CSV headers
    file << "Move Number,FEN,Move\n";

    // Write each position and move to CSV
    int moveNumber = 1;
    for (const auto& entry : gameData) {
        file << moveNumber << "," << entry.first << "," << entry.second << "\n";
        moveNumber++;
    }

    file.close();
    std::cout << "Game saved to " << filename << std::endl;
}


std::vector<std::string> alphazero_predict(std::vector<SPG*>* spGames, std::shared_ptr<MCTS> mcts, c10::ScalarType precision)
{
    auto results = mcts->predict(spGames, true);
    
    std::vector<std::string> move_results;

    for (int i = 0; i < spGames->size(); i++)
    {
        torch::Tensor action_probs = std::get<0>(results.at(i));
        float state_value = std::get<1>(results.at(i));

        auto temp_indexs = torch::nonzero(action_probs);
        
        torch::Tensor sampled_index = torch::multinomial(action_probs.view({-1}), 1, true);
        
        action_probs = torch::zeros({8 * 8 * 73}, torch::TensorOptions().dtype(precision));
        action_probs[sampled_index.item<int>()] = 1.0f;

        action_probs = action_probs.view({8, 8, 73});

        std::string  action = spGames->at(i)->game->decode_action(spGames->at(i)->current_state, action_probs);
        
        move_results.push_back(action);
    }

    return move_results;
}

std::vector<std::string> heuristic_predict(std::vector<SPG*>* spGames, int depth, bool quien)
{

    std::vector<std::tuple<int,std::string>> move_results;

    #pragma omp parallel for default(none) shared(spGames, move_results, depth)
    for (int i = 0; i < spGames->size(); i++)
    {
        spGames->at(i)->game->m_Board->reset_leaf_nodes();
        float eval = spGames->at(i)->game->m_Board->alpha_beta(depth, -1000000, 1000000, quien);
        int move_int = spGames->at(i)->game->m_Board->get_bot_best_move();
        
        std::string move = spGames->at(i)->game->m_Board->move_to_uci(move_int);
        

        move_results.push_back(std::make_tuple(i, move));

    }

    std::sort(move_results.begin(), move_results.end(), [](const auto& a, const auto& b) {
        return std::get<0>(a) < std::get<0>(b);
    });

    std::vector<std::string> final_results;
    for (const auto& [num, str] : move_results) {
        final_results.push_back(str);
    }

    return final_results;
}

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
    int depth = config.value("depth", 2);
    int n_games = config.value("n_games", 2);
    std::string model_path = config.value("model_path", "");
    std::string device = config.value("device", "cpu");
    std::string precision_type = config.value("precision_type", "float32");
    std::string log_path = config.value("log_path", "");

    c10::ScalarType precision;    

    if (precision_type == "float32")
    {
        std::cout << "Input type float 32" << std::endl;
        precision = torch::kFloat32;
    }
    else if (precision_type == "float16")
    {
        std::cout << "Input type float 16" << std::endl;
        precision = torch::kBFloat16;
    }
    else
    {
        std::cout << "Input type float 32" << std::endl;
        precision = torch::kFloat32;
    }

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
    m_ResNetChess->to(precision);
    std::cout << model_path << std::endl;
    if (model_path != "")
    {
        load_Resnet(model_path, m_ResNetChess, m_Device);
    }
    
    auto mcts = std::make_shared<MCTS>(m_ResNetChess, num_searches, dichirlet_alpha, dichirlet_epsilon, C, precision);

    std::vector<SPG*> spGamesW;
    std::vector<SPG*> spGamesB;

    int alpha_wins = 0;
    int minmax_wins = 0;

    for (int i = 0; i < n_games; i++)
    {
        auto game = std::make_shared<Game>();
        game->m_Board->parse_fen(initial_position.c_str());
        SPG* spg = new SPG(game, 0.0);
        spGamesW.push_back(spg);
    }
    for (int i = 0; i < n_games; i++)
    {
        auto game = std::make_shared<Game>();
        game->m_Board->parse_fen(initial_position.c_str());
        SPG* spg = new SPG(game, 0.0);
        spGamesB.push_back(spg);
    }

    int move_counter = 0;

    spGamesW.at(0)->game->m_Board->print_board();

    while (spGamesW.size() > 0) // AlphaZero is white - MinMax is black
    {
        std::vector<std::string> moves;
        // std::cout << "Move " << std::to_string(move_counter + 1) << " Num Games: " << std::to_string(spGamesW.size()) <<std::endl; 
        if (move_counter % 2 == 0) // First Player
        {
            // std::cout << "AlphaZero Search" << std::endl;        
            moves = alphazero_predict(&spGamesW, mcts, precision);
            
        }
        else // Second Player
        {
            // std::cout << "MinMax Search" << std::endl;
            moves = heuristic_predict(&spGamesW, depth, true);
        }

        // std::cout << "Next State" << std::endl;
        for (int i = 0; i < spGamesW.size(); i++)
        {
            final_state fs;
            
            
            // std::cout << i << std::endl;
            // std::cout << "Get Move" << std::endl;
            
            auto action = moves.at(i);
            // std::cout << "Next State" << std::endl;
            fs = spGamesW.at(i)->game->get_next_state_and_value(spGamesW.at(i)->current_state, action, spGamesW.at(i)->repeated_states);
            // 
            // std::cout << "Repeated states" << std::endl;
            spGamesW.at(i)->repeated_states[fs.board_state.bitboards] += 1;
            
            // std::cout << "Half moves" << std::endl;
            if (fs.board_state.halfmove == 0) 
            {
                spGamesW.at(i)->repeated_states.clear(); // Impossible to repeat states if piece captured or pawn moved
            }
            
            // std::cout << "Delete Root" << std::endl;

            if (move_counter % 2 == 0)
            {
                delete spGamesW.at(i)->pRoot;
            }
            
            // std::cout << "Check final" << std::endl;
            if (fs.terminated)
            {
                std::cout << "Ending a game with value " << std::to_string(fs.value) << std::endl;
                if (move_counter % 2 == 0 && fs.value != 0)
                {
                    alpha_wins++;
                }
                else if (fs.value != 0)
                {
                    minmax_wins++;
                }
                // std::cout << "Erasing the last game" << std::endl;
                spGamesW.erase(spGamesW.begin() + i);
                // std::cout << "Last game erased" << std::endl;

            }
            else
            {
                spGamesW.at(i)->current_state = fs.board_state;
                spGamesW.at(i)->game->set_state(fs.board_state);
                

                if (i == 0)
                {
                    if (move_counter % 2 == 0)
                    {
                        std::cout << "----------- Alpha Play -----------------" << std::endl;
                    }
                    else
                    {
                        std::cout << "----------- MinMax Play -----------------" << std::endl;
                    }
                    
                    spGamesW.at(0)->game->m_Board->print_board();
                    std::cout << moves.at(0) << std::endl;
                }
            }

        }


        move_counter++;
    }
    std::cout << "Change board sides" <<std::endl;
    move_counter = 0;

    while (spGamesB.size() > 0) // AlphaZero is black - MinMax is white
    {
        std::vector<std::string> moves;
        // std::cout << "Move " << std::to_string(move_counter + 1) << " Num Games: " << std::to_string(spGamesB.size()) <<std::endl; 
       
        if (move_counter % 2 == 0) 
        {
            // std::cout << "MinMax" << std::endl;
            
            moves = heuristic_predict(&spGamesB, depth, true);
        }
        else
        {
            // std::cout << "Alphazero" << std::endl;
            moves = alphazero_predict(&spGamesB, mcts, precision);
            
        }

        // std::cout << "Next State" << std::endl;
        for (int i = 0; i < spGamesB.size(); i++)
        {
            final_state fs;
            
            // std::cout << i << std::endl;
            // std::cout << "Get Move" << std::endl;
            
            auto action = moves.at(i);
            // std::cout << "Next State" << std::endl;
            fs = spGamesB.at(i)->game->get_next_state_and_value(spGamesB.at(i)->current_state, action, spGamesB.at(i)->repeated_states);
            // 
            // std::cout << "Repeated states" << std::endl;
            spGamesB.at(i)->repeated_states[fs.board_state.bitboards] += 1;
            
            // std::cout << "Half moves" << std::endl;
            if (fs.board_state.halfmove == 0) 
            {
                spGamesB.at(i)->repeated_states.clear(); // Impossible to repeat states if piece captured or pawn moved
            }
            
            // std::cout << "Delete Root" << std::endl;

            if (move_counter % 2 != 0)
            {
                delete spGamesB.at(i)->pRoot;
            }
            
            // std::cout << "Check final" << std::endl;
            if (fs.terminated)
            {
                std::cout << "Ending a game with value " << std::to_string(fs.value) << std::endl;
                
                if (move_counter % 2 == 0 && fs.value != 0)
                {
                    minmax_wins++;
                }
                else if (fs.value != 0)
                {
                    alpha_wins++;
                }
                spGamesB.erase(spGamesB.begin() + i);
            }
            else
            {
                spGamesB.at(i)->current_state = fs.board_state;
                spGamesB.at(i)->game->set_state(fs.board_state);
            }

        }
        move_counter++;
    }

    std::cout << "Alpha Wins percentage: "  << std::to_string((alpha_wins / (2.0f * n_games)) * 100) << std::endl;
    std::cout << "Minmax Wins percentage: "  << std::to_string((minmax_wins / (2.0f * n_games)) * 100) << std::endl;
    std::cout << "Draw percentage: "  << std::to_string((((2.0f * n_games) - minmax_wins - alpha_wins) / (2.0f * n_games)) * 100) << std::endl;

    return 0;
}