#include <iostream>
// #include <torch/torch.h>
// #include "include/network.h"
// #include "include/ResNet.h"
#include "mcts.h"
#include "AlphaZero.h"
#include "game.h"

// using namespace torch;

int main()
{

    // ResNetChess model(5, 256, torch::kCPU);

    // std::cout << model << std::endl;

    // Tensor x;
    // x = torch::randn({2, 19, 8, 8});

    // chess_output output = model.forward(x);
    // std::cout << output.policy << std::endl;

    // BitBoard board;
    // BitBoard* board_ptr = &board;

    // board.parse_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPpP/R3K2R w KQkq - 0 1");
    // board.parse_fen("7K/q7/7k/8/8/8/8/8 b - - 99 1");
    // board.parse_fen(start_position);
    // state board_state;
    // copy_state_from_board(board_state, board_ptr);
    // std::vector<SPG*> spGames;

    // for (int i = 0; i < 150; i++)
    // {
    //     SPG* spg = new SPG(&game);
    //     spGames.push_back(spg);
    // }

    // MCTS mcts(600, 0.03, 0.25, 1.41);
    // std::array<std::size_t, 4> shape = {1, 8, 8, 73}; 
    // xt::xtensor<float, 4> encoded_moves(shape);
    // xt::xtensor<float, 4> encoded_move(shape);
    // encoded_moves.fill(0.0f);
    Game game;
    int start = get_time_ms();


    // mcts.search(&spGames);
    // board_state.side = 1;
    // game.get_encoded_state(board_state);
    // board.print_board();
    // encoded_moves = game.get_valid_moves_encoded(board_state);
    // encoded_moves(0, 6, 3, 9) = 2;
    // game.decode_actions(board_state, encoded_moves);
    

    AlphaZero az(&game, 5, 1, 1, 1, 10, 12, 1, 0.0003, 1.3, 0.8, 1.41, 0.0001, 7);

    std::vector<sp_memory_item> memory;
    xt::xtensor<float, 3> encoded_state;

    sp_memory_item m;

    // memory.push_back(m);
    // memory.push_back(m);
    // memory.push_back(m);
    // memory.push_back(m);
    // memory.push_back(m);
    // memory.push_back(m);
    // memory.push_back(m);
    // memory.push_back(m);
    // memory.push_back(m);
    // memory.push_back(m);
    // memory.push_back(m);
    // memory.push_back(m);
    // memory.push_back(m);
    // memory.push_back(m);
    // memory.push_back(m);
    // memory.push_back(m);
    // memory.push_back(m);
    // memory.push_back(m);
    // memory.push_back(m);
    // az.train(memory);
    az.learn();

    int end = get_time_ms();

    std::cout << "Time taken: " << end - start << "ms" << std::endl;

    // encoded_move = game.get_encoded_action("d4e6", 1);

    // game.get_value_and_terminated(board_state, "a7b7");

    // int count = 0;
    // for (int i = 0; i < 73; i++)
    // {
    //     std::cout << "\nNew Plane " << i  << std::endl;
    //     for (int j = 0; j < 8; j++)
    //     {
    //         for (int k = 0; k < 8; k++)
    //         {
    //             std::cout << encoded_moves(0, j, k, i) << " ";
    //             if (encoded_moves(0, j, k, i) == 1)
    //             {
    //                 count++;
    //             }
    //         }
    //         std::cout << std::endl;
    //     }
    //     std::cout << std::endl;
    //     getchar();
    // }

    // std::cout << "Total moves: " << count << std::endl;

    // Game game;

    // game.get_encoded_state(board_state);

    // game.get_next_state(board_state, "e2e4");

    // board.print_bitboard(board_state.bitboards[0]);
    
}