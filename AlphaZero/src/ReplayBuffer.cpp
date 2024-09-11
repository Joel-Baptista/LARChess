#include "ReplayBuffer.h"

ReplayBuffer::ReplayBuffer(int buffer_size)
{
    this->buffer_size = buffer_size;
    this->pos = 0;
    this->full = false;
    this->states = torch::zeros({buffer_size, 19, 8, 8});
    this->action_probs = torch::zeros({buffer_size, 8, 8, 73});
    this->values = torch::zeros({buffer_size});
    this->current_game_id = 0;

    int* pGameIds = new int[buffer_size];
}

ReplayBuffer::~ReplayBuffer()
{
    delete[] pGameIds;
}

int ReplayBuffer::size()
{
    return this->full ? this->buffer_size : this->pos;
}

void ReplayBuffer::add(torch::Tensor encoded_state, torch::Tensor action_probs, torch::Tensor value)
{
    int index = this->pos % this->buffer_size;

    this->states[index] = encoded_state;
    this->action_probs[index] = action_probs;
    this->values[index] = value.item<float>();
    this->pGameIds[index] = current_game_id;

    this->pos++;   
}

void ReplayBuffer::reset()
{
    this->pos = 0;
    this->full = false;
}

buffer_items ReplayBuffer::sample(int batch_size)
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
        std::cout << "idx: " << idx << std::endl;
        
        if (random_indices.find(idx) == random_indices.end() && game_counts[idx] <= 30)  // Check if the index is already in the set
        {
            random_indices.insert(idx);  // Insert it (only if it's not already in the set)
            game_counts[idx]++;
        }
    }

    buffer_items samples;
    samples.states = torch::zeros({batch_size, 19, 8, 8});
    samples.action_probs = torch::zeros({batch_size, 8, 8, 73});
    samples.values = torch::zeros({batch_size});

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