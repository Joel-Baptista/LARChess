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

    std::vector<std::vector<SPG*>> spGames_vec;
    spGames_vec.push_back(m_spGames1);
    spGames_vec.push_back(m_spGames2);
    spGames_vec.push_back(m_spGames3);
    spGames_vec.push_back(m_spGames4);

    std::string filename = "../" + std::to_string(n_games) + ".csv";

    std::ifstream file(filename);

    // If it exists, delete the file
    file.close();
    if (std::remove(filename.c_str()) != 0) {
        std::cerr << "Error deleting log file: " << filename << std::endl;
    } else {
        std::cout << "Previous log file deleted: " << filename << std::endl;
    }



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
    std::vector<SPG*> spGames;
    evalResults results;

    m_ResNetSwarm.at(thread_id)->eval();

    games.at(thread_id)->m_Board->parse_fen(start_position);
    SPG* spg = new SPG(games.at(thread_id), 0.0);
    spGames.push_back(spg);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    double random_number = dis(gen);

    int alpha_white;
    if (random_number < 0.5)
    {
        alpha_white = 1;
    }
    else
    {
        alpha_white = 0;
    }

    int eval_res_sum;

    while (true)
    {
        for (int k=0; k < 4; k++)
        {
            if (spGames.at(0)->game->m_Board->get_side() == alpha_white)
            {
                std::vector<std::vector<std::tuple<at::Tensor, float>>> results_per_spGames;

                #pragma omp parallel for
                for (int j = 0; j < 4; j++)
                {
                    auto results = mcts_vector.at(j)->predict( &spGames_vec.at(j));
                    
                    #pragma omp atomic
                    {
                        results_per_spGames.push_back(results);
                    }
                }

                auto results = results_per_spGames.at(k);
                auto m_spGames = spGames_vec.at(k);

                #pragma omp parallel for
                for (int i = 0; i < m_spGames.size(); i++)
                {
                    torch::Tensor action_probs = std::get<0>(results.at(i));
                    float state_value = std::get<1>(results.at(i));

                    action_probs = action_probs.view({8, 8, 73});

                    std::string  action = m_spGames.at(i)->game->decode_action(m_spGames.at(i)->current_state, action_probs);
                }

            }

        }
        else
        {
            games.at(thread_id)->m_Board->reset_leaf_nodes();
            float eval = games.at(thread_id)->m_Board->alpha_beta(5, -1000000, 1000000, true);
            games.at(thread_id)->m_Board->make_bot_move(games.at(thread_id)->m_Board->get_bot_best_move());
        }
        
        final_state fs;
        // std::cout << "Action: " << action << std::endl;
        fs = m_spGames.at(i)->game->get_next_state_and_value(m_spGames.at(i)->current_state, action, m_spGames.at(i)->repeated_states);

        m_spGames.at(i)->repeated_states[fs.board_state.bitboards] += 1;

        if (fs.board_state.halfmove == 0) 
        {
            m_spGames.at(i)->repeated_states.clear(); // Impossible to repeat states if piece captured or pawn moved
        }

        delete m_spGames.at(i)->pRoot;

        if (fs.terminated)
        {
            if ((m_spGames.at(i)->current_state.side == alpha_white) && (fs.value == 1.0))
            {
                eval_res_sum = eval_res_sum - 1;
            }
            else if ((m_spGames.at(i)->current_state.side != alpha_white) && (fs.value == 1.0))
            {
                eval_res_sum = eval_res_sum + 1;
            }
        }
        else
        {
            m_spGames.at(i)->current_state = fs.board_state;
            m_spGames.at(i)->game->set_state(fs.board_state);
        }

        state current_state = games.at(thread_id)->get_state();
        spGames.at(0)->current_state = current_state;

        final_state fState = spGames.at(0)->game->get_value_and_terminated(current_state, spGames.at(0)->repeated_states);

        if (fState.terminated)
        {
            int res;
            if ((current_state.side == alpha_white) && (fState.value == 1.0))
            {
                res = -1;
            }
            else if ((current_state.side != alpha_white) && (fState.value == 1.0))
            {
                res = 1;
            }
            else
            {
                res = 0;
            }
            delete spGames.at(0);
            spGames.erase(spGames.begin());
            return res;
        }
    }


    return 0;
}
