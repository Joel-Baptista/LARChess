#include <iostream>
#include <array>
#include <vector>
#include <string>
#include <tuple>
#include "minmax.h"

// Value based on position of the piece
int counter = 0;

std::array<std::array<double, 8>, 8> center_control = {
    { { 0.8, 0.8,  0.8,  0.8,  0.8,  0.8, 0.8, 0.8},
      { 0.8, 0.9,  0.9,  0.9,  0.9,  0.9, 0.9, 0.8 },
      { 0.8, 0.9, 0.95, 0.95, 0.95, 0.95, 0.9, 0.8 },
      { 0.8, 0.9, 0.95,    1,    1, 0.95, 0.9, 0.8 },
      { 0.8, 0.9, 0.95,    1,    1, 0.95, 0.9, 0.8 },
      { 0.8, 0.9, 0.95, 0.95, 0.95, 0.95, 0.9, 0.8 },
      { 0.8, 0.9,  0.9,  0.9,  0.9,  0.9, 0.9, 0.8 },
      { 0.8, 0.8,  0.8,  0.8,  0.8,  0.8, 0.8, 0.8 } }
};

std::tuple<std::string, double> minmax(Board board, int depth, bool maximizing_player, double alpha, double beta){
    counter++;
    // std::cout << "Counter: " << counter << std::endl;
    if (depth == 0 || board.is_terminal()){
        return {board.get_fen(), evaluate(board)};
    }

    board.get_all_legal_moves();

    // std::cout << board.legal_moves["g8"].size() << std::endl;
    double target_eval = maximizing_player ? -1000000 : 1000000;
    std::string best_move;

    int legal_moves_n = 0;
    for (auto const& [square, moves] : board.legal_moves){
        legal_moves_n += moves.size();
    }

    std::cout << "Depth: " << depth << " Legal moves: " << legal_moves_n << std::endl;

    if (maximizing_player){
        for (auto const& [square, moves] : board.legal_moves){
            for (int i=0; i<moves.size(); i++){
                Board new_board = board;
                new_board.make_move(moves[i]);
                
                std::string new_fen;
                double eval;
                std::tie(new_fen, eval) = minmax(new_board, depth-1, false, alpha, beta);
                if (eval > target_eval){
                    target_eval = eval;
                    best_move = moves[i].getFrom() + moves[i].getTo();
                }
                alpha = std::max(alpha, eval);
                if (beta <= alpha){
                    break;
                }
            }
        }
        // std::cout << "Depth: " << depth << " Max eval: " << target_eval << std::endl;
        return {best_move, target_eval};
    }else{
        for (auto const& [square, moves] : board.legal_moves){
            for (int i=0; i<moves.size(); i++){
                Board new_board = board;
                new_board.make_move(moves[i]);
                std::string new_fen;
                double eval;
                std::tie(new_fen, eval) = minmax(new_board, depth-1, true, alpha, beta);
                if (eval < target_eval){
                    target_eval = eval;
                    best_move = moves[i].getFrom() + moves[i].getTo();
                }
                beta = std::min(beta, eval);
                if (beta <= alpha){
                    break;
                }
            }
        }

        // std::cout << "Depth: " << depth << " Max eval: " << target_eval << std::endl;
        return {best_move, target_eval};
    }
}


double evaluate(Board& board){
    double score = 0.0;

    if (board.is_checkmate()){
        return (board.turn_player == 1) ? -1000 : 1000;
    }

    for (int i=0; i<8; i++){
        for (int j=0; j<8; j++){
            score += piece_value(board.board[i][j]) * center_control[i][j];
        }
    }
    
    return score;

}

double piece_value(int piece){
    switch (piece)
    {
    case 1:
        return 1.0;
    case 2:
        return 3.0;
    case 3:
        return 3.0;
    case 4:
        return 5.0;
    case 5:
        return 9.0;
    case 6:
        return 0.0;
    case -1:
        return -1.0;
    case -2:
        return -3.0;
    case -3:
        return -3.0;
    case -4:
        return -5.0;
    case -5:
        return -9.0;
    case -6:
        return 0.0;
    default:
        return 0.0;
    }
}