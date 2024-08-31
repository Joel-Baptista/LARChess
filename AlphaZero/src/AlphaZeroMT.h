#pragma once

#include <memory>
#include "mcts.h"
#include "game.h"

#include <iostream>
#include <thread>
#include <future>
#include <vector>

class AlphaZeroMT
{
    public:
        AlphaZeroMT(
                    int num_searches, 
                    int num_iterations, 
                    int num_selfPlay_iterations, 
                    int num_parallel_games, 
                    int num_epochs, 
                    int batch_size, 
                    float temperature, 
                    float temperature_decay, 
                    float temperature_min, 
                    float learning_rate, 
                    float dichirlet_alpha, 
                    float dichirlet_epsilon, 
                    float dichirlet_epsilon_decay, 
                    float dichirlet_epsilon_min, 
                    float C,
                    float C_decay,
                    float C_min,
                    int num_evals,
                    int depth,
                    float weight_decay,
                    int num_resblocks,
                    int num_channels,
                    std::string device,
                    std::string pretrained_model_path,
                    int num_threads
                    );

        ~AlphaZeroMT();

        void learn();
        void train(std::vector<sp_memory_item> memory);
        void save_model(std::string path);
        void load_model(std::string path);
        void save_model() { save_model(""); }
        void load_model() { load_model(""); }

        std::string predict(std::shared_ptr<Game> games);

    private:

        std::vector<sp_memory_item> SelfPlay(int thread_id);
        int AlphaEval(int thread_id, int depth);

        std::vector<memory_item> memory;

        void update_dichirlet();
        void update_temperature();
        void update_C();

        std::shared_ptr<ResNetChess> m_ResNetChess;
        std::unique_ptr<torch::optim::Adam> m_Optimizer;
        std::shared_ptr<torch::Device> m_Device;
        
        std::vector<std::shared_ptr<ResNetChess>> m_ResNetSwarm;
        std::vector<std::shared_ptr<MCTS>> m_mcts;

        std::vector<std::shared_ptr<Game>> games;

        int num_searches;
        int num_iterations;
        int num_selfPlay_iterations;
        int num_parallel_games;
        int num_epochs;
        int batch_size;
        float temperature;
        float temperature_decay;
        float temperature_min;
        float learning_rate;
        float dichirlet_alpha;
        float dichirlet_epsilon;
        float dichirlet_epsilon_decay;
        float dichirlet_epsilon_min;
        float C;
        float C_decay;
        float C_min;
        int num_evals;
        int depth;
        float weight_decay;
        int num_resblocks;
        int num_channels;
        int num_threads;

        std::string log_file;

};  
