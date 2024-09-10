#include "ReplayBuffer.h"

ReplayBuffer::ReplayBuffer(int buffer_size)
{
    this->buffer_size = buffer_size;
    this->pos = 0;
    this->full = false;
    this->states = torch::zeros({buffer_size, 19, 8, 8});
    this->action_probs = torch::zeros({buffer_size, 8, 8, 73});
    this->values = torch::zeros({buffer_size});
}

ReplayBuffer::~ReplayBuffer()
{
}

int ReplayBuffer::size()
{
    return this->full ? this->buffer_size : this->pos;
}

void ReplayBuffer::add(torch::Tensor encoded_state, torch::Tensor action_probs, float value)
{
    int index = this->pos % this->buffer_size;

    this->states[index] = encoded_state;
    this->action_probs[index] = action_probs;
    this->values[index] = value;

    this->pos++;   
}

void ReplayBuffer::reset()
{
    this->pos = 0;
    this->full = false;
}

std::vector<buffer_item> ReplayBuffer::sample(int batch_size)
{

    // Random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, size() - 1);

    // Use a set to store unique random indices
    std::unordered_set<int> random_indices;

    while (random_indices.size() < batch_size) {
        int idx = distrib(gen);  // Generate a random index
        
        if (random_indices.find(idx) == random_indices.end())  // Check if the index is already in the set
        {
            random_indices.insert(idx);  // Insert it (only if it's not already in the set)
        }
    }

    std::vector<buffer_item> samples;

    for (int idx : random_indices)
    {
        buffer_item sample;
        sample.state = this->states[idx];
        sample.action_probs = this->action_probs[idx];
        sample.value = this->values[idx];

        samples.push_back(sample);
    }

    return samples;
}