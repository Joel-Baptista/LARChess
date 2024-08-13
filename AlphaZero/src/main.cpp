#include <iostream>
// #include <torch/torch.h>
// #include "include/network.h"
// #include "include/ResNet.h"
#include "mcts.h"
#include "AlphaZero.h"

// using namespace torch;

int main()
{

    // ResNetChess model(5, 256, torch::kCPU);

    // std::cout << model << std::endl;

    // Tensor x;
    // x = torch::randn({2, 19, 8, 8});

    // chess_output output = model.forward(x);
    // std::cout << output.policy << std::endl;

    BitBoard board;
    board.parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    std::vector<SPG*> spGames;

    for (int i = 0; i < 150; i++)
    {
        SPG* spg = new SPG(&board);
        spGames.push_back(spg);
    }

    MCTS mcts(1200, 0.03, 0.25, 1.41);

    mcts.search(&spGames);

}