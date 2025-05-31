#include <iostream>
#include <string>
#include "bit_board.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <cassert>
#include <omp.h>

int main(int argc, char* argv[]) {
    BitBoard bit_board;

    // std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/2P6/PP1PPPPP/RNBQKBNR b KQkq - 0 1";

    bit_board.parse_fen(fen.c_str());

    moves move_list[1]; 

    bit_board.get_moves(move_list);
    
    std::cout << "Total moves generated: " << move_list->count << std::endl;

    // auto cores = omp_get_num_procs();
    int cores = 1; // Default value
    int depth = 5; // Default depth value
    if (argc > 1) {
        try {
            cores = std::stoi(argv[1]);
        } catch (const std::exception& e) {
            std::cerr << "Invalid input for cores, defaulting to 1." << std::endl;
        }
    }
    if (argc > 2) {
        try {
            depth = std::stoi(argv[2]);
        } catch (const std::exception& e) {
            std::cerr << "Invalid input for depth, defaulting to 5." << std::endl;
        }
    }
    std::cout << "Number of cores: " << cores << std::endl;
    std::cout << "Depth: " << depth << std::endl;

    std::vector<moves> move_lists(cores);
    std::vector<BitBoard> bit_boards(cores);

    for (int i = 0; i < cores; i++)
    {
        bit_boards[i].parse_fen(fen.c_str());
    }


    for (int i = 0; i < cores; i++)
    {
        move_lists[i].count = 0;
    }

    for (int i = 0; i < move_list->count; i++)
    {
        
        int core_id = i % cores; // Distribute moves across cores 
        move_lists[core_id].moves[move_lists[core_id].count] = move_list->moves[i];
        move_lists[core_id].count++;

    }

    int st = get_time_ms();

    std::vector<std::tuple<float, std::string>> evaluations(cores);

    #pragma omp parallel for 
    for (int i = 0; i < cores; i++)
    {
        auto eval = bit_boards[i].alpha_beta(&move_lists[i], depth, -1000000, 1000000, true);

        std::cout << "Core " << i << " evaluation: " << eval << "Move: " << bit_boards[i].move_to_uci(bit_boards[i].get_bot_best_move()) << std::endl;
        evaluations[i] = std::make_tuple(eval, bit_boards[i].move_to_uci(bit_boards[i].get_bot_best_move()));
    }

    float best_eval = std::get<0>(evaluations[0]);
    std::string best_move = std::get<1>(evaluations[0]);
    for (int i = 1; i < cores; i++)
    {
        if (bit_board.get_side() == 0 && std::get<0>(evaluations[i]) > best_eval)
        {
            best_eval = std::get<0>(evaluations[i]);
            best_move = std::get<1>(evaluations[i]);
        }
        else if (bit_board.get_side() == 1 && std::get<0>(evaluations[i]) < best_eval)
        {
            best_eval = std::get<0>(evaluations[i]);
            best_move = std::get<1>(evaluations[i]);
        }
    }
    std::cout << "Best evaluation: " << best_eval << " Best move: " << best_move << std::endl;

    int end = get_time_ms();
    std::cout << "Time taken: " << end - st << " ms" << std::endl;


    return 0;
}