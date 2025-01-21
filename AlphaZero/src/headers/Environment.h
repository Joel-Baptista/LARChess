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

class Environment
{
    public:
        Environment(
            int id,
            std::shared_ptr<ResNetChess> model,
            std::shared_ptr<MCTS> mcts,
            int num_parallel_games,
            std::shared_ptr<ReplayBuffer> buffer,
            float early_stopping,
            float early_stopping_value
        );
        ~Environment();

        void step();

    private:
        std::vector<SPG*> m_spGames;
        std::shared_ptr<ResNetChess> m_Model;
        std::shared_ptr<MCTS> m_Mcts;
        int m_num_parallel_games;
        float m_early_stopping;
        float m_early_stopping_value;
        std::shared_ptr<Logger> m_Logger;
        std::shared_ptr<ReplayBuffer> m_Buffer;
        std::vector<std::shared_ptr<Game>> m_Games;

};