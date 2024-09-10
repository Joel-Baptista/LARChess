#include "game.h"


struct buffer_item
{
    torch::Tensor state;
    torch::Tensor action_probs;
    torch::Tensor value;
};

class ReplayBuffer
{
    public:
        ReplayBuffer(int buffer_size);
        ~ReplayBuffer();
    
    private:

        int pos;
        bool full;
        int buffer_size;
        torch::Tensor states;
        torch::Tensor action_probs;
        torch::Tensor values;
        std::unique_ptr<int> indices;

        int size();
        void add(torch::Tensor encoded_state, torch::Tensor action_probs, float value);
        void reset();
        std::vector<buffer_item> sample(int batch_size);
};