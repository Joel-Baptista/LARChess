#pragma once

#include <memory>
#include "mcts.h"
#include "game.h"


class AlphaZero
{
    public:
        AlphaZero(Game* game,
                  int num_searches, 
                  int num_iterations, 
                  int num_selfPlay_iterations, 
                  int num_parallel_games, 
                  int num_epochs, 
                  int batch_size, 
                  float temperature, 
                  float learning_rate, 
                  float dichirlet_alpha, 
                  float dichirlet_epsilon, 
                  float C,
                  float weight_decay,
                  int num_resblocks);

        ~AlphaZero();

        void learn();
        void train(std::vector<sp_memory_item> memory);
    private:

        std::vector<sp_memory_item> SelfPlay();

        std::vector<memory_item> memory;


        ResNetChess* m_ResNetChess;
        std::unique_ptr<torch::optim::Adam> m_Optimizer;
        std::unique_ptr<torch::Device> m_Device;
        
        std::unique_ptr<MCTS> m_mcts;

        Game* game;

        int num_searches;
        int num_iterations;
        int num_selfPlay_iterations;
        int num_parallel_games;
        int num_epochs;
        int batch_size;
        float temperature;
        float learning_rate;
        float dichirlet_alpha;
        float dichirlet_epsilon;
        float C;
        float weight_decay;
        int num_resblocks;
};  
