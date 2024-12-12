#include "../../src/headers/alphabot.h"
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

    AlphaBot bot(std::make_shared<Game>(), 
                 num_searches, 
                 C, 
                 temperature,
                 dichirlet_alpha, 
                 dichirlet_epsilon, 
                 num_resblocks, 
                 num_channels, 
                 device);

    if (model_path != "")
        bot.load_model(model_path);
        
    bot.m_Game->m_Board->parse_fen(initial_position.c_str());

    while (true)
    {
        bot.m_Game->m_Board->print_board();
        if (bot.m_Game->m_Board->get_side() == !(human_player == "w" ? 0 : 1))
        {   
            // std::cout << "Bot is thinking..." << std::endl;
            std::string move = bot.predict();
            std::cout << "Bot move: " << move << std::endl;
            getchar();
            bot.make_bot_move(move);
            // std::cout << "Bot move made" << std::endl;
        }
        else
        {
            moves move_list;
            bot.m_Game->m_Board->get_generate_moves(&move_list);
            
            std::cout << "Moves: [";
            for (int i = 0; i < move_list.count; i++)
            {
                auto move = move_list.moves[i];
                std::cout << bot.m_Game->m_Board->move_to_uci(move) << ", ";

            }
            std::cout << "]" << std::endl;

            std::string gui_move;
            std::cout << "Enter your move: ";
            std::cin >> gui_move;

            bot.m_Game->m_Board->make_player_move(gui_move.c_str());
        }
    }
    return 0;
}