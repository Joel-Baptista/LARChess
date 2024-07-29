#include <iostream>
#include "../src/board.h"
#include "../src/move.h"
#include "../src/utils.h"
#include <random>
// #include <chrono>
// #include <algorithm>


Move get_random_move(std::unordered_map<std::string, std::vector<Move>> all_legal_moves);
std::vector<Move> flatten_moves(std::unordered_map<std::string, std::vector<Move>> all_legal_moves);
Move human_move();

int main() {
    Board board;

    // board.set_from_fen("8/1r2Bp1r/k7/pp1pR1PN/3P2nP/N7/5Q2/4K2q w  - 5 49");
    bool human_vs_human = false;
    bool human_white = true;
    bool machine_vs_machine = true;

    if (human_vs_human){
        machine_vs_machine = false;
    }

    std::cout << "Starting position" << std::endl;
    while (!board.is_terminal()){
        
        if (!machine_vs_machine){
            board.show();
        }
        std::cout << "FEN: " << board.get_fen() << std::endl;
        board.get_all_legal_moves();
        Move player_move;

        // std::cout << "Hash of board = " << board.get_board_hash() << std::endl;
        
        if (board.turn_player == 1){
            std::string move_input;
            
            std::cout << "White to move" << std::endl;
            
            if (machine_vs_machine){
                player_move = get_random_move(board.legal_moves);
            }else if (human_vs_human){
                player_move = human_move();
            }else if (!human_vs_human && human_white){
                player_move = human_move();
            }else{
                player_move = get_random_move(board.legal_moves);
            }
            // player_move = get_random_move(board.legal_moves);
            // std::cout << "White move: " << player_move.getFrom() << player_move.getTo() << std::endl;

        }else if (board.turn_player == -1){
            std::cout << "Black to move" << std::endl;

            if (machine_vs_machine){
                player_move = get_random_move(board.legal_moves);
            }else if (human_vs_human){
                player_move = human_move();
            }else if (!human_vs_human && !human_white){
                player_move = human_move();
            }else{
                player_move = get_random_move(board.legal_moves);
            }


            // player_move = get_random_move(board.legal_moves);
            // std::cout << "Black move: " << player_move.getFrom() << player_move.getTo() << std::endl;
        }

        board.make_move(player_move);
    }

    std::cout << "Final position: " << board.get_fen() << std::endl;
    board.show();

    std::cout << "Game over" << std::endl;

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

Move human_move(){
    std::string move_input;
    std::cout << "Enter move: ";
    std::cin >> move_input;

    if (move_input.length() == 5){
        Move move(move_input.substr(0,2), move_input.substr(2,2), move_input[4]);
        return move;
    }else{
        Move move(move_input.substr(0,2), move_input.substr(2,2));
        return move;
    }
}