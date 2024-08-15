#pragma once

#include <torch/torch.h>
#include "include/ResNet.h"
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
    private:

        void SelfPlay();

        std::vector<memory_item> memory;


        std::unique_ptr<ResNetChess> m_ResNetChess;
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
