#include <iostream>
#include <torch/torch.h>
#include "include/network.h"
#include "include/ResNet.h"
#include "mcts.h"
#include "AlphaZeroV2.h"
#include "game.h"

#include "include/json.hpp"

// Stockfish

// #include "include/stockfish/src/bitboard.h"
// #include "include/stockfish/src/movegen.h"
// #include "include/stockfish/src/position.h"
// #include "include/stockfish/src/search.h"
// #include "include/stockfish/src/thread.h"
// #include "include/stockfish/src/uci.h"
// #include "include/stockfish/src/ucioption.h"
// #include <boost/process.hpp>


using json = nlohmann::json;
// namespace bp = boost::process;

int main()
{


    // // Path to Stockfish executable
    // std::string stockfish_path = "/home/joel/projects/YACE/AlphaZero/src/include/stockfish/src/stockfish";
    
    // // Create a child process
    // bp::ipstream out_stream; // Pipe for reading Stockfish's output
    // bp::opstream in_stream;  // Pipe for sending input to Stockfish
    // bp::child stockfish_process(stockfish_path, bp::std_out > out_stream, bp::std_in < in_stream);

    // // Initialize Stockfish (UCI mode)
    // in_stream << "uci" << std::endl;

    // // Read Stockfish's initialization response
    // std::string line;
    // while (out_stream && std::getline(out_stream, line) && line != "uciok") {
    //     std::cout << line << std::endl;
    // }

    // // Send custom commands to Stockfish
    // std::string user_input;
    // while (true) {
    //     std::cout << "Enter command (or 'quit' to exit): ";
    //     std::getline(std::cin, user_input);

    //     if (user_input == "quit") {
    //         break;
    //     }

    //     // Send the user input to Stockfish
    //     in_stream << user_input << std::endl;

    //     // Read and print Stockfish's response
    //     while (std::getline(out_stream, line) && !line.empty()) {
    //         std::cout << line << std::endl;

    //         if (line.find("bestmove") != std::string::npos) {
    //             std::cout << "Stockfish's move: " << line << std::endl;
    //             break;
    //         }
    //     }
    //     std::cout << "I'm here" << std::endl;
    // }

    // // Clean up
    // in_stream << "quit" << std::endl; // Send quit command to Stockfish
    // stockfish_process.wait();          // Wait for the process to finish

    

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

    // Extract values from the JSON object
    int num_searches_init = config.value("num_searches_init", 0);
    int num_searches_max = config.value("num_searches_max", 0);
    float num_searches_ratio = config.value("num_searches_ratio", 0.0);
    int search_depth = config.value("search_depth", 0);
    int num_iterations = config.value("num_iterations", 0);
    int warmup_iters = config.value("warmup_iters", 0);
    int num_selfPlay_iterations = config.value("num_selfPlay_iterations", 0);
    int num_parallel_games = config.value("num_parallel_games", 0);
    int num_epochs = config.value("num_epochs", 0);
    int max_state_per_game = config.value("max_state_per_game", 0);
    int swarm_update_freq = config.value("swarm_update_freq", 0);
    int batch_size = config.value("batch_size", 0);
    float early_stopping = config.value("early_stopping", 0.0);
    float early_stopping_value = config.value("early_stopping_value", 0.0);
    int buffer_size = config.value("buffer_size", 0);
    double temperature = config.value("temperature", 0.0);
    double temperature_decay = config.value("temperature_decay", 0.0);
    double temperature_min = config.value("temperature_min", 0.0);
    double learning_rate_innit = config.value("learning_rate_innit", 0.0);
    double learning_rate_decay = config.value("learning_rate_decay", 0.0);
    double learning_rate_min = config.value("learning_rate_min", 0.0);
    double learning_rate_update_freq = config.value("learning_rate_update_freq", 0.0);
    double dichirlet_alpha = config.value("dichirlet_alpha", 0.0);
    double dichirlet_epsilon = config.value("dichirlet_epsilon", 0.0);
    double dichirlet_epsilon_decay = config.value("dichirlet_epsilon_decay", 0.0);
    double dichirlet_epsilon_min = config.value("dichirlet_epsilon_min", 0.0);
    double C = config.value("C", 1.41);
    double C_min = config.value("C_min", 1.41);
    double C_decay = config.value("C_decay", 0.995);
    int num_evals = config.value("num_evals", 1);
    int eval_freq = config.value("eval_freq", 1000);
    int depth = config.value("depth", 4);
    double weight_decay = config.value("weight_decay", 0.0);
    double dropout = config.value("dropout", 0.0);
    double gradient_clip = config.value("gradient_clip", 0.0);
    int num_resblocks = config.value("num_resblocks", 0);
    int num_channels = config.value("num_channels", 0);
    std::string model_name = config.value("model_name", "default_model");
    std::string device = config.value("device", "cpu");
    std::string pretrained_model_path = config.value("pretrained_model_path", "cpu");
    int threads = config.value("threads", 1);
    std::string precision_type = config.value("precision", "float32");
    bool debug = config.value("debug", false);

    Game game;
    int start = get_time_ms();    

    AlphaZeroV2 az(  
                num_searches_init,  
                num_searches_max, 
                num_searches_ratio,
                search_depth,      
                num_iterations,       
                warmup_iters,       
                num_selfPlay_iterations,
                num_parallel_games,
                num_epochs,   
                max_state_per_game, 
                swarm_update_freq,    
                batch_size, 
                early_stopping,
                early_stopping_value,
                buffer_size,       
                temperature,      
                temperature_decay,      
                temperature_min,      
                learning_rate_innit,
                learning_rate_decay,
                learning_rate_min,
                learning_rate_update_freq,    
                dichirlet_alpha,  
                dichirlet_epsilon,
                dichirlet_epsilon_decay,
                dichirlet_epsilon_min,
                C,      
                C_decay,      
                C_min,      
                num_evals,
                eval_freq,
                depth,
                weight_decay, 
                dropout, 
                gradient_clip,
                num_resblocks,
                num_channels,
                device,
                pretrained_model_path,
                threads,
                precision_type,
                debug
                );       
    // AlphaZero az(&game,     
    //              num_searches,         
    //              num_iterations,       
    //              num_selfPlay_iterations,
    //              num_parallel_games,
    //              num_epochs,        
    //              batch_size,        
    //              temperature,      
    //              learning_rate,    
    //              dichirlet_alpha,  
    //              dichirlet_epsilon,
    //              dichirlet_epsilon_decay,
    //              dichirlet_epsilon_min,
    //              C,      
    //              weight_decay, 
    //              num_resblocks,
    //             num_channels,
    //             device                
    //             );       

    az.learn();

    int end = get_time_ms();

    std::cout << "Time taken: " << end - start << "ms" << std::endl;

    
}