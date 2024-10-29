#pragma once

#include "../../../BBChessEngine/src/bit_board.h"
#include "mcts.h"
#include <vector>
#include "game.h"
#include <array>
#include <memory>
#include <unordered_map>

class MCTSMT
{
    public:
        MCTSMT(int num_searches, float dichirlet_alpha, float dichirlet_epsilon, float C, int num_threads, int num_resblocks, int num_channels, std::string device, std::string pretrained_model_path);
        ~MCTSMT();


        int get_num_searches() { return num_searches; }

        void search(std::vector<SPG*>* spGames, int thread_id);
        std::vector<std::tuple<torch::Tensor, float>> predict(std::vector<SPG*>* spGames);

        // void search(std::vector<SPG*>* spGames) { std::vector<c10::cuda::CUDAStream> cuda_streams; search(spGames, cuda_streams); }

        void set_dichirlet_epsilon(float epsilon) { dichirlet_epsilon = epsilon; }
        void set_C(float c) { C = c; }
        void set_num_searches(int s) { num_searches = s; }
        std::unordered_map<BitboardKey, int, BitboardHash> get_boards_visited() { return boards_visited; }
        
        void load_model(std::shared_ptr<ResNetChess> model, std::string path);

    private:
        int num_searches;
        float dichirlet_alpha;
        float dichirlet_epsilon;
        int search_depth;
        float C;

        int num_threads;
        int num_resblocks;
        int num_channels;

        int current_search;

        std::shared_ptr<torch::Device> m_Device;
        std::vector<std::shared_ptr<ResNetChess>> m_ModelSwarm;

        std::unordered_map<BitboardKey, int, BitboardHash> boards_visited;
        std::mutex mtxBackProp;
        std::mutex mtxSelect;
        std::mutex mtxIncreaseSearch;


};


