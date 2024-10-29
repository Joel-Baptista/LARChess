#include <iostream>
#include <torch/torch.h>
#include "../include/network.h"
#include "../include/ResNet.h"
#include "mcts.h"
#include "mctsMT.h"
#include "AlphaZeroV2.h"
#include "game.h"
#include "utils.h"

#include "../include/json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

int main()
{

    std::ifstream config_file("../config_test_mctsMT.json");
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
    std::cout << pretrained_model_path << std::endl;
    if (pretrained_model_path != "")
    {
        load_Resnet(pretrained_model_path, m_ResNetChess, m_Device);
    }

    auto mcts = std::make_unique<MCTS>(m_ResNetChess, num_searches_init, dichirlet_alpha, dichirlet_epsilon, C);
    auto mctsMT = std::make_unique<MCTSMT>(num_searches_init, dichirlet_alpha, dichirlet_epsilon, C, num_threads, num_resblocks, num_channels, device, pretrained_model_path);
    auto game = std::make_shared<Game>();
    auto gameMT = std::make_shared<Game>();

    std::vector<SPG*> m_spGames;

    game->m_Board->parse_fen(start_position);
    SPG* spg = new SPG(game);
    m_spGames.push_back(spg);

    std::vector<SPG*> m_spGamesMT;

    game->m_Board->parse_fen(start_position);
    SPG* spgMT = new SPG(gameMT);
    m_spGamesMT.push_back(spgMT);

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


    for (int i = 0;  i < chessFiles.size(); i++)
    {

        if (position_type == "puzzle")
        {
            auto chessPositions = readPuzzleCSV(directory_path + "/" + chessFiles.at(i));
            
            for (auto& chessPosition : chessPositions)
            {
                std::cout << "File: " << chessFiles.at(i) << std::endl;
                game->m_Board->parse_fen(chessPosition.epd.c_str());
                m_spGames[0]->reset();
                m_spGames[0]->current_state = game->get_state();
                gameMT->m_Board->parse_fen(chessPosition.epd.c_str());
                m_spGamesMT[0]->reset();
                m_spGamesMT[0]->current_state = gameMT->get_state();

                size_t pos = chessPosition.move.find(' ');

                // Get the first word
                std::string pre_move = chessPosition.move.substr(0, pos);

                auto state_next = game->get_next_state(m_spGames[0]->current_state, pre_move);                
                m_spGames[0]->current_state = state_next;
                game->set_state(state_next);
                
                auto state_nextMT = gameMT->get_next_state(m_spGamesMT[0]->current_state, pre_move);                
                m_spGamesMT[0]->current_state = state_nextMT;
                gameMT->set_state(state_nextMT);

                game->m_Board->print_board();
                gameMT->m_Board->print_board();

                std::cout << "Rating: " << chessPosition.Rating << std::endl;
                std::cout << "Theme: " << chessPosition.Themes << std::endl;

                auto st = get_time_ms();

                auto results = mcts->predict(&m_spGames);
                std::string  action = m_spGames.at(0)->game->decode_action(m_spGames.at(0)->current_state, std::get<0>(results.at(0)));
                auto eval = std::get<1>(results.at(0));

                std::cout << "Time taken: " << (get_time_ms() - st) / 1000.0f << "seconds, unique boards visited: " << mcts->get_boards_visited().size() << std::endl;
                std::cout << "Best move: " << chessPosition.move << " Predicted move: " << action << " Predicted value: " << eval << std::endl;

                auto resultsMT = mctsMT->predict(&m_spGamesMT);
                std::string  actionMT = m_spGamesMT.at(0)->game->decode_action(m_spGamesMT.at(0)->current_state, std::get<0>(resultsMT.at(0)));
                auto evalMT = std::get<1>(resultsMT.at(0));

                std::cout << "Time taken: " << (get_time_ms() - st) / 1000.0f << "seconds, unique boards visited: " << mctsMT->get_boards_visited().size() << std::endl;
                std::cout << "Best move: " << chessPosition.move << " Predicted move: " << actionMT << " Predicted value: " << evalMT << std::endl;

                auto eval_shaped = (eval / abs(eval)) * std::pow(abs(eval), alpha) * 150.0f;
                std::cout << std::endl;
                getchar();
            }
        }
        else
        {
            auto chessPositions = readChessCSV(directory_path + "/" + chessFiles.at(i));
            auto chessPosition = chessPositions.at(0);
            std::cout << "File: " << chessFiles.at(i) << std::endl;
            game->m_Board->parse_fen(chessPosition.epd.c_str());
            m_spGames[0]->reset();
            m_spGames[0]->current_state = game->get_state();

            game->m_Board->print_board();

            auto st = get_time_ms();

            auto results = mcts->predict(&m_spGames);

            std::cout << "Time taken: " << (get_time_ms() - st) / 1000.0f << "seconds" << std::endl;

            std::string  action = m_spGames.at(0)->game->decode_action(m_spGames.at(0)->current_state, std::get<0>(results.at(0)));
            auto eval = std::get<1>(results.at(0));

            auto eval_shaped = (eval / abs(eval)) * std::pow(abs(eval), alpha) * 150.0f;

            std::cout << "Best move: " << chessPosition.move << " Predicted move: " << action << std::endl;
            std::cout << "Eval: " << chessPosition.eval << " Predicted eval: " << eval_shaped << std::endl;
            std::cout << std::endl;
            for (int j = 0;  j < chessPositions.size(); j++)
            {
                std::cout << "Move #" << j + 1 << ": " << chessPositions.at(j).move << ", eval: " << chessPositions.at(j).eval;

                if (chessPositions.at(j).move == action)
                {
                    std::cout << " (Predicted)";
                }

                std::cout << std::endl;
            }
        }


        getchar();
    }

    return 0;
}
