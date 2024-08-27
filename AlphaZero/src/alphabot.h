#include "game.h"
#include "mcts.h"


class AlphaBot
{
    public:
        AlphaBot(std::shared_ptr<Game> game, 
                   int num_searches, 
                   float C, 
                   float temp,
                   float dichirlet_alpha, 
                   float dichirlet_epsilon, 
                   int num_resblocks,
                   int num_channels,
                   std::string device);
        ~AlphaBot();

        std::string predict();
        void load_model(std::string path);
        void make_bot_move(std::string move);
        std::shared_ptr<Game> m_Game;

    private:

    int m_NumSearches;
    float m_C;
    float m_temp;
    std::shared_ptr<torch::Device> m_Device;
    std::shared_ptr<ResNetChess> m_ResNetChess;
    std::unique_ptr<MCTS> m_mcts;
};