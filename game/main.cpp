#include <iostream>
#include "../src/board.h"
#include "../src/move.h"
#include "../src/utils.h"
#include <random>
// #include <chrono>
// #include <algorithm>


Move get_random_move(std::unordered_map<std::string, std::vector<Move>> all_legal_moves);
std::vector<Move> flatten_moves(std::unordered_map<std::string, std::vector<Move>> all_legal_moves);

int main() {
    Board board;

    std::cout << "Starting position" << std::endl;
    while (true){
        board.show();
        board.get_all_legal_moves();
        Move player_move;
        
        if (board.turn_player == 1){
            std::string move_input;
            
            std::cout << "White to move" << std::endl;
            std::cin >> move_input;

            Move move(move_input.substr(0,2), move_input.substr(2,2));
            player_move = move;

        }else if (board.turn_player == -1){
            std::cout << "Black to move" << std::endl;

            player_move = get_random_move(board.legal_moves);

            std::cout << "Black move: " << player_move.getFrom() << player_move.getTo() << std::endl;
        }

        board.make_move(player_move);
        std::cout << "----------------------------------------------" << std::endl;
        
    }

}

Move get_random_move(std::unordered_map<std::string, std::vector<Move>> all_legal_moves){

    std::vector<Move> legal_moves_flattened = flatten_moves(all_legal_moves);
    std::random_device rd;  // Obtain a random number from hardware
    std::mt19937 gen(rd()); // Seed the generator
    std::uniform_int_distribution<> distr(0, legal_moves_flattened.size()-1);

    return legal_moves_flattened[distr(gen)];
}

std::vector<Move> flatten_moves(std::unordered_map<std::string, std::vector<Move>> all_legal_moves){
    std::vector<Move> moves;
    int count = 0;
    for (auto const& [key, value] : all_legal_moves){
        moves.insert(moves.end(), value.begin(), value.end());

        // for (int i=0; i<value.size(); i++){
        //     std::cout << value[i].getFrom() << value[i].getTo() << std::endl;
        // }

    }
    return moves;
}