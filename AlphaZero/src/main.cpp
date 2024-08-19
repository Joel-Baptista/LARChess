#include <iostream>
// #include <torch/torch.h>
// #include "include/network.h"
// #include "include/ResNet.h"
#include "mcts.h"
#include "AlphaZero.h"
#include "game.h"

#include "include/json.hpp"

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

    // Extract values from the JSON object
    int num_searches = config.value("num_searches", 0);
    int num_iterations = config.value("num_iterations", 0);
    int num_selfPlay_iterations = config.value("num_selfPlay_iterations", 0);
    int num_parallel_games = config.value("num_parallel_games", 0);
    int num_epochs = config.value("num_epochs", 0);
    int batch_size = config.value("batch_size", 0);
    double temperature = config.value("temperature", 0.0);
    double learning_rate = config.value("learning_rate", 0.0);
    double dichirlet_alpha = config.value("dichirlet_alpha", 0.0);
    double dichirlet_epsilon = config.value("dichirlet_epsilon", 0.0);
    double dichirlet_epsilon_decay = config.value("dichirlet_epsilon_decay", 0.0);
    double dichirlet_epsilon_min = config.value("dichirlet_epsilon_min", 0.0);
    double C = config.value("C", 0.0);
    double weight_decay = config.value("weight_decay", 0.0);
    int num_resblocks = config.value("num_resblocks", 0);
    int num_channels = config.value("num_channels", 0);
    std::string model_name = config.value("model_name", "default_model");

    Game game;
    int start = get_time_ms();    

    AlphaZero az(&game,     
                 num_searches,         
                 num_iterations,       
                 num_selfPlay_iterations,
                 num_parallel_games,
                 num_epochs,        
                 batch_size,        
                 temperature,      
                 learning_rate,    
                 dichirlet_alpha,  
                 dichirlet_epsilon,
                 dichirlet_epsilon_decay,
                 dichirlet_epsilon_min,
                 C,      
                 weight_decay, 
                 num_resblocks,
                num_channels
                );       

    az.learn();

    int end = get_time_ms();

    std::cout << "Time taken: " << end - start << "ms" << std::endl;

    
}