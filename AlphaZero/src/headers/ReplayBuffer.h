#pragma once
#include "game.h"
#include <mutex>

struct buffer_items
{
    torch::Tensor states;
    torch::Tensor action_probs;
    torch::Tensor values;
};

class ReplayBuffer
{
    public:
        ReplayBuffer(int buffer_size, int stat_window);
        ~ReplayBuffer();
        void add(torch::Tensor encoded_state, torch::Tensor action_probs, torch::Tensor value);
        void add_stats(torch::Tensor res, torch::Tensor len);
        float get_mean_len() { return torch::mean(stats_lens).cpu().item<float>();}
        float get_mean_res() { return torch::mean(stats_res).cpu().item<float>();}
        void reset();
        void adding_new_game() { current_game_id++; }
        int get_current_game_id() { return current_game_id; }
        buffer_items sample(int batch_size, int max_state_per_game);
        int size();
        std::mutex mtxAddBuffer;
    
    private:

        int pos;
        bool full;
        int buffer_size;
        torch::Tensor states;
        torch::Tensor action_probs;
        torch::Tensor values;
        int* pGameIds;
        int current_game_id;

        torch::Tensor stats_lens;
        torch::Tensor stats_res;
        int stats_pos;
        bool stats_full;
        int stat_window;
 
};