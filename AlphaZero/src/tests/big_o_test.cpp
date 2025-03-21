#include <iostream>
#include <torch/torch.h>
#include "../include/network.h"
#include "../include/ResNet.h"
#include "mcts.h"
#include "mctsMT.h"
#include "AlphaZeroV2.h"
#include "game.h"
#include "utils.h"
#include <omp.h>

#include "../include/json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

int main()
{

    std::ifstream config_file("../config_test_big_o.json");
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

    std::string device = config.value("device", "cpu");
    std::string pretrained_model_path =  config.value("pretrained_model_path", "");
    std::string directory_path = config.value("directory_path", "../../../analysis");
    std::string position_type = config.value("position_type", "puzzle");
    int num_resblocks = config.value("num_resblocks", 0);
    int num_channels = config.value("num_channels", 0);
    int num_searches_init = config.value("num_searches_init", 0);
    float dichirlet_alpha = config.value("dichirlet_alpha", 0.0);
    float dichirlet_epsilon = config.value("dichirlet_epsilon", 0.0);
    float C = config.value("C", 0.0);
    float alpha = config.value("alpha", 0.0);
    int num_threads = config.value("num_threads", 0);
    int n_games = config.value("n_games", 0);
    int delta_games = config.value("delta_games", 0);

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
    
    auto m_ResNetChess1 = std::make_shared<ResNetChess>(num_resblocks, num_channels, 0.0, *m_Device);
    m_ResNetChess1->to(*m_Device, torch::kFloat32);
    std::cout << pretrained_model_path << std::endl;
    if (pretrained_model_path != "")
    {
        load_Resnet(pretrained_model_path, m_ResNetChess1, m_Device);
    }
    auto m_ResNetChess2 = std::make_shared<ResNetChess>(num_resblocks, num_channels, 0.0, *m_Device);
    m_ResNetChess2->to(*m_Device, torch::kFloat32);
    std::cout << pretrained_model_path << std::endl;
    if (pretrained_model_path != "")
    {
        load_Resnet(pretrained_model_path, m_ResNetChess2, m_Device);
    }
    auto m_ResNetChess3 = std::make_shared<ResNetChess>(num_resblocks, num_channels, 0.0, *m_Device);
    m_ResNetChess3->to(*m_Device, torch::kFloat32);
    std::cout << pretrained_model_path << std::endl;
    if (pretrained_model_path != "")
    {
        load_Resnet(pretrained_model_path, m_ResNetChess3, m_Device);
    }
    auto m_ResNetChess4 = std::make_shared<ResNetChess>(num_resblocks, num_channels, 0.0, *m_Device);
    m_ResNetChess3->to(*m_Device, torch::kFloat32);
    std::cout << pretrained_model_path << std::endl;
    if (pretrained_model_path != "")
    {
        load_Resnet(pretrained_model_path, m_ResNetChess4, m_Device);
    }

    auto mcts1 = std::make_shared<MCTS>(m_ResNetChess1, num_searches_init, dichirlet_alpha, dichirlet_epsilon, C);
    auto mcts2 = std::make_shared<MCTS>(m_ResNetChess2, num_searches_init, dichirlet_alpha, dichirlet_epsilon, C);
    auto mcts3 = std::make_shared<MCTS>(m_ResNetChess3, num_searches_init, dichirlet_alpha, dichirlet_epsilon, C);
    auto mcts4 = std::make_shared<MCTS>(m_ResNetChess4, num_searches_init, dichirlet_alpha, dichirlet_epsilon, C);
    auto game = std::make_shared<Game>();

    std::vector<std::shared_ptr<MCTS>> mcts_vector;
    mcts_vector.push_back(mcts1);
    mcts_vector.push_back(mcts2);
    mcts_vector.push_back(mcts3);
    mcts_vector.push_back(mcts4);

    std::vector<std::string> chessFiles;

    try {
        // Iterate over the directory entries
        for (const auto& entry : fs::directory_iterator(directory_path)) {
            // Check if the entry is a regular file and if it starts with "index"
            if (entry.is_regular_file() && entry.path().filename().string().rfind("index", 0) == 0) {
                // Print the name of the file
                chessFiles.push_back(entry.path().filename().string());
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error accessing the directory: " << e.what() << std::endl;
    }


    std::cout << "File: " << chessFiles.at(0) << std::endl;
    auto chessPositions = readPuzzleCSV(directory_path + "/" + chessFiles.at(0));
    auto chessPosition = chessPositions.at(0);

    std::vector<SPG*> m_spGames1;
    std::vector<SPG*> m_spGames2;
    std::vector<SPG*> m_spGames3;
    std::vector<SPG*> m_spGames4;

    std::vector<std::vector<SPG*>> spGames;
    spGames.push_back(m_spGames1);
    spGames.push_back(m_spGames2);
    spGames.push_back(m_spGames3);
    spGames.push_back(m_spGames4);

    std::string filename = "../" + std::to_string(n_games) + ".csv";

    std::ifstream file(filename);

    // If it exists, delete the file
    file.close();
    if (std::remove(filename.c_str()) != 0) {
        std::cerr << "Error deleting log file: " << filename << std::endl;
    } else {
        std::cout << "Previous log file deleted: " << filename << std::endl;
    }

    logMessage("n_games,time,ratio", filename);

    for (int i = 0;  i < n_games / delta_games; i++)
    {
        game->m_Board->parse_fen(start_position);

        for (int k = 0; k < (delta_games / 4); k++)
        {
            for (int j = 0; j < 4; j++)
            {
                auto spg_game = std::make_shared<Game>();
                SPG* spg = new SPG(spg_game, 0.0);
                spGames.at(j).push_back(spg);
            }
        }

        game->m_Board->parse_fen(chessPosition.epd.c_str());

        for (int j = 0; j < spGames.at(0).size(); j++)
        {
            for (int w = 0; w < 4; w++)
            {
                spGames.at(w).at(j)->reset();
                spGames.at(w).at(j)->current_state = game->get_state();
                size_t pos = chessPosition.move.find(' ');

                // Get the first word
                std::string pre_move = chessPosition.move.substr(0, pos);

                auto state_next = game->get_next_state(spGames.at(w).at(j)->current_state, pre_move);                
                spGames.at(w).at(j)->current_state = state_next;
                game->set_state(state_next);
            }
        }

        std::cout << "Number of games running in paralel: " << spGames.at(0).size() << std::endl;

        auto st = get_time_ms();

        #pragma omp parallel for
        for (int j = 0; j < 4; j++)
        {
            // std::cout << "Started MCTS " << j + 1 << std::endl;
            auto results = mcts_vector.at(j)->predict( &spGames.at(j));
        }

        auto time_taken = (float)(get_time_ms() - st) / 1000.0f;
        auto ratio = time_taken / ((i + 1)* delta_games);
        
        std::cout << "Time taken for " << (i + 1)* delta_games << " games : " << time_taken << " ratio: " << ratio << std::endl;
        logMessage(std::to_string((i + 1)* delta_games) + "," + std::to_string(time_taken) + "," + std::to_string(ratio), filename);
        std::cout << "--------------------------------------------------------" << std::endl;
      
    }
    

    // for (int i = 0;  i < chessFiles.size(); i++)
    // {

    //     if (position_type == "puzzle")
    //     {
    //         auto chessPositions = readPuzzleCSV(directory_path + "/" + chessFiles.at(i));
            
    //         for (auto& chessPosition : chessPositions)
    //         {
    //             std::cout << "File: " << chessFiles.at(i) << std::endl;
    //             game->m_Board->parse_fen(chessPosition.epd.c_str());
    //             m_spGames[0]->reset();
    //             m_spGames[0]->current_state = game->get_state();
    //             gameMT->m_Board->parse_fen(chessPosition.epd.c_str());
    //             m_spGamesMT[0]->reset();
    //             m_spGamesMT[0]->current_state = gameMT->get_state();

    //             size_t pos = chessPosition.move.find(' ');

    //             // Get the first word
    //             std::string pre_move = chessPosition.move.substr(0, pos);

    //             auto state_next = game->get_next_state(m_spGames[0]->current_state, pre_move);                
    //             m_spGames[0]->current_state = state_next;
    //             game->set_state(state_next);
                
    //             auto state_nextMT = gameMT->get_next_state(m_spGamesMT[0]->current_state, pre_move);                
    //             m_spGamesMT[0]->current_state = state_nextMT;
    //             gameMT->set_state(state_nextMT);

    //             game->m_Board->print_board();
    //             gameMT->m_Board->print_board();

    //             std::cout << "Rating: " << chessPosition.Rating << std::endl;
    //             std::cout << "Theme: " << chessPosition.Themes << std::endl;

    //             auto st = get_time_ms();

    //             auto results = mcts->predict(&m_spGames);
    //             std::string  action = m_spGames.at(0)->game->decode_action(m_spGames.at(0)->current_state, std::get<0>(results.at(0)));
    //             auto eval = std::get<1>(results.at(0));

    //             std::cout << "Time taken: " << (get_time_ms() - st) / 1000.0f << "seconds, unique boards visited: " << mcts->get_boards_visited().size() << std::endl;
    //             std::cout << "Best move: " << chessPosition.move << " Predicted move: " << action << " Predicted value: " << eval << std::endl;

    //             auto resultsMT = mctsMT->predict(&m_spGamesMT);
    //             std::string  actionMT = m_spGamesMT.at(0)->game->decode_action(m_spGamesMT.at(0)->current_state, std::get<0>(resultsMT.at(0)));
    //             auto evalMT = std::get<1>(resultsMT.at(0));

    //             std::cout << "Time taken: " << (get_time_ms() - st) / 1000.0f << "seconds, unique boards visited: " << mctsMT->get_boards_visited().size() << std::endl;
    //             std::cout << "Best move: " << chessPosition.move << " Predicted move: " << actionMT << " Predicted value: " << evalMT << std::endl;

    //             auto eval_shaped = (eval / abs(eval)) * std::pow(abs(eval), alpha) * 150.0f;
    //             std::cout << std::endl;
    //             getchar();
    //         }
    //     }
    //     else
    //     {
    //         auto chessPositions = readChessCSV(directory_path + "/" + chessFiles.at(i));
    //         auto chessPosition = chessPositions.at(0);
    //         std::cout << "File: " << chessFiles.at(i) << std::endl;
    //         game->m_Board->parse_fen(chessPosition.epd.c_str());
    //         m_spGames[0]->reset();
    //         m_spGames[0]->current_state = game->get_state();

    //         game->m_Board->print_board();

    //         auto st = get_time_ms();

    //         auto results = mcts->predict(&m_spGames);

    //         std::cout << "Time taken: " << (get_time_ms() - st) / 1000.0f << "seconds" << std::endl;

    //         std::string  action = m_spGames.at(0)->game->decode_action(m_spGames.at(0)->current_state, std::get<0>(results.at(0)));
    //         auto eval = std::get<1>(results.at(0));

    //         auto eval_shaped = (eval / abs(eval)) * std::pow(abs(eval), alpha) * 150.0f;

    //         std::cout << "Best move: " << chessPosition.move << " Predicted move: " << action << std::endl;
    //         std::cout << "Eval: " << chessPosition.eval << " Predicted eval: " << eval_shaped << std::endl;
    //         std::cout << std::endl;
    //         for (int j = 0;  j < chessPositions.size(); j++)
    //         {
    //             std::cout << "Move #" << j + 1 << ": " << chessPositions.at(j).move << ", eval: " << chessPositions.at(j).eval;

    //             if (chessPositions.at(j).move == action)
    //             {
    //                 std::cout << " (Predicted)";
    //             }

    //             std::cout << std::endl;
    //         }
    //     }


    //     getchar();
    // }

    return 0;
}
