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

    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -";
    // std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

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
    std::cout << "Perft depth: " << depth << std::endl;

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

    long long leaf_nodes = 0;

    #pragma omp parallel for reduction(+:leaf_nodes)
    for (int i = 0; i < cores; i++)
    {
        bit_boards[i].perft_driver(depth, &move_lists[i]);
        leaf_nodes += bit_boards[i].get_leaf_nodes();
        std::cout << "Core " << i << " leaf nodes: " << bit_boards[i].get_leaf_nodes() << std::endl;
    }

    std::cout << "Total leaf nodes: " << leaf_nodes << std::endl;
    int end = get_time_ms();
    std::cout << "Time taken: " << end - st << " ms" << std::endl;


    return 0;
}