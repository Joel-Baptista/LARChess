#pragma once

#include <memory>
#include "mcts.h"
#include "game.h"
#include "ReplayBuffer.h"
#include "logger.h"

#include <iostream>
#include <thread>
#include <future>
#include <vector>
#include <mutex>


struct evalResults
{
    int win_count = 0;
    int loss_count = 0;
    int draw_count = 0;
};

class AlphaZeroMT
{
    public:
        AlphaZeroMT(             
                    int num_searches_init, 
                    int num_searches_max,  
                    float num_search_ratio,
                    int search_depth,
                    int num_iterations, 
                    int num_selfPlay_iterations, 
                    int num_parallel_games, 
                    int num_epochs, 
                    int max_state_per_game,
                    int swarm_update_freq,
                    int batch_size, 
                    int buffer_size,
                    float temperature, 
                    float temperature_decay, 
                    float temperature_min, 
                    float learning_rate_innit,
                    float learning_rate_decay,
                    float learning_rate_min,
                    float learning_rate_update_freq, 
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
                    float dropout,
                    float gradient_clip,
                    int num_resblocks,
                    int num_channels,
                    std::string device,
                    std::string pretrained_model_path,
                    int num_threads,
                    bool debug
                    );

        ~AlphaZeroMT();

        void learn();
        void train();
        void save_model(std::string path);
        void load_model(std::string path);
        void save_model() { save_model(""); }
        void load_model() { load_model(""); }

        std::string predict(std::shared_ptr<Game> games);

    private:

        void SelfPlay(int thread_id);
        int AlphaEval(int thread_id, int depth);

        void log(std::string message);
        void logTrain(std::string message);
        void logEval(std::string message);
        void logConfig();
        int train_iter = 0;
        int eval_iter = 0;

        std::vector<memory_item> memory;

        void update_dichirlet();
        void update_temperature();
        void update_C();
        void update_num_searches();
        void update_learning_rate();
        
        void update_hyper();

        void network_sanity_check(ResNetChess& source, ResNetChess& target);

        std::shared_ptr<ResNetChess> m_ResNetChess;
        std::unique_ptr<torch::optim::SGD> m_Optimizer;
        std::shared_ptr<torch::Device> m_Device;

        std::vector<c10::cuda::CUDAStream> cuda_streams;
        
        std::vector<std::shared_ptr<ResNetChess>> m_ResNetSwarm;
        std::vector<std::shared_ptr<MCTS>> m_mcts;

        std::vector<std::shared_ptr<Game>> games;
        std::unique_ptr<ReplayBuffer> m_Buffer; 
        std::mutex mtxBuffer;
        std::unique_ptr<Logger> m_logger; 

        int num_searches;
        int num_searches_init; 
        int num_searches_max; 
        float num_searches_ratio;
        int search_depth;
        int num_iterations;
        int num_selfPlay_iterations;
        int num_parallel_games;
        int num_epochs;
        int max_state_per_game;
        int swarm_update_freq;
        int batch_size;
        int buffer_size;
        float temperature;
        float temperature_decay;
        float temperature_min;
        float learning_rate;
        float learning_rate_innit;
        float learning_rate_decay;
        float learning_rate_min;
        float learning_rate_update_freq; 
        int lr_last_update;
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
        float dropout;
        float gradient_clip;
        int num_resblocks;
        int num_channels;
        int num_threads;
        bool debug;

        std::string log_file;
        std::string model_path;

};  
