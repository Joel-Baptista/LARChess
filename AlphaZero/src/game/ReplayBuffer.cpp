#include "ReplayBuffer.h"

ReplayBuffer::ReplayBuffer(int buffer_size, c10::ScalarType precision, int stat_window = 100)
{
    this->precision = precision;
    this->buffer_size = buffer_size;
    this->pos = 0;
    this->full = false;
    this->states = torch::zeros({buffer_size, 19, 8, 8}, precision);
    this->action_probs = torch::zeros({buffer_size, 8, 8, 73}, precision);
    this->values = torch::zeros({buffer_size}, precision);
    this->current_game_id = 0;

    pGameIds = new int[buffer_size];

    this->stats_lens = torch::zeros({stat_window});
    this->stats_res = torch::zeros({stat_window});
    this->stats_pos = 0;
    this->stats_full = false;
    this->stat_window = stat_window;
}

ReplayBuffer::~ReplayBuffer()
{
    delete[] pGameIds;
}

void ReplayBuffer::add_stats(torch::Tensor res, torch::Tensor len)
{
    int index = this->stats_pos % this->stat_window;

    this->stats_lens[index] = len;
    this->stats_res[index] = res;
    
    this->stats_pos++;   

    if (this->stats_pos > this->stat_window) this->stats_full = true;
}

int ReplayBuffer::size()
{
    return this->full ? this->buffer_size : this->pos;
}

void ReplayBuffer::add(torch::Tensor encoded_state, torch::Tensor action_probs, torch::Tensor value)
{
    int index = this->pos % this->buffer_size;

    this->states[index] = encoded_state[0];
    this->action_probs[index] = action_probs[0];
    this->values[index] = value.item<float>();
    this->pGameIds[index] = current_game_id;

    this->pos++;   

    if (this->pos > this->buffer_size) full = true;
}

void ReplayBuffer::reset()
{
    this->pos = 0;
    this->full = false;
}

buffer_items ReplayBuffer::sample(int batch_size, int max_state_per_game)
{

    // Random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, size() - 1);

    // Use a set to store unique random indices
    std::unordered_set<int> random_indices;
    std::unordered_map<int, int> game_counts;

    while (random_indices.size() < batch_size) {
        int idx = distrib(gen);  // Generate a random index
        
        if (random_indices.find(idx) == random_indices.end() && game_counts[idx] <= max_state_per_game)  // Check if the index is already in the set
        {
            random_indices.insert(idx);  // Insert it (only if it's not already in the set)
            game_counts[idx]++;
        }
    }

    buffer_items samples;
    samples.states = torch::zeros({batch_size, 19, 8, 8}, precision);
    samples.action_probs = torch::zeros({batch_size, 8, 8, 73}, precision);
    samples.values = torch::zeros({batch_size}, precision);

    int count = 0;
    for (int idx : random_indices)
    {
        samples.states[count] = this->states[idx];
        samples.action_probs[count] = this->action_probs[idx];
        samples.values[count] = this->values[idx];
        
        count++;
    }

    return samples;
}