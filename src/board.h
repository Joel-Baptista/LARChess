#ifndef BOARD_H
#define BOARD_H

#include <string>
#include <vector>
#include <array>
#include "move.h"

class Board {
public:
    Board();
    std::string start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::string fen;
    std::array<std::array<int, 8>, 8> board;
    void show();
    void reset();
    char turn();
    bool castling_rights(char player, char side);
    void board_from_fen(const std::string& fen);
    void fen_from_board(const std::array<std::array<int, 8>, 8> board);

    void make_move(Move move);

    enum Piece{
        W_PAWN = 1,
        W_KNIGHT = 2,
        W_BISHOP = 3,
        W_ROOK = 4,
        W_QUEEN = 5,
        W_KING = 6,
        B_PAWN = -1,
        B_KNIGHT = -2,
        B_BISHOP = -3,
        B_ROOK = -4,
        B_QUEEN = -5,
        B_KING = -6,
        EMPTY = 0
    };

private:

};

#endif // BOARD_H