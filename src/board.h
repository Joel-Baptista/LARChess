#ifndef BOARD_H
#define BOARD_H

#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include "move.h"

class Board {
public:
    Board();
    Board(std::string fen_init);
    std::string start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::array<std::array<int, 8>, 8> board;
    int turn_player; // 1 for white, -1 for black
    std::array<bool, 4> castling_rights = {false, false, false, false}; // 0 for white king side, 1 for white queen side, 2 for black king side, 3 for black queen side
    std::string en_passant;
    int halfmove_clock;
    int fullmove_number;
    std::unordered_map<std::string, std::vector<Move>> legal_moves;
    bool legal_moves_calculated = false;
    
    void show();
    void reset();
    
    void show_from_fen();
    int update_turn_from_fen();
    void update_castling_rights_from_fen();
    void update_en_passant_from_fen();
    void update_halfmove_clock_from_fen();
    void update_fullmove_number_from_fen();
    void update_fen();
    std::string get_fen();
    void set_fen(std::string new_fen);

    // bool castling_rights(char player, char side);
    void update_board_from_fen(const std::string& fen);
    void update_fen_from_board(const std::array<std::array<int, 8>, 8> board);

    void make_move(Move move);
    std::vector<Move> get_legal_moves(std::string square);
    std::unordered_map<std::string, std::vector<Move>> get_all_legal_moves();
    

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
    std::string fen;
    std::vector<Move> get_pawn_moves(int row, int col);
    std::vector<Move> get_knight_moves(int row, int col);
    std::vector<Move> get_bishop_moves(int row, int col);
    std::vector<Move> get_rook_moves(int row, int col);
    std::vector<Move> get_queen_moves(int row, int col);
    std::vector<Move> get_king_moves(int row, int col);

    std::array<std::array<int, 2>, 8> knight_moves = { // Clockwise 8 possible knight moves starting from bottom left
        {
            {-2, -1}, 
            {-1, -2}, 
            {1, -2}, 
            {2, -1}, 
            {2, 1}, 
            {1, 2}, 
            {-1, 2}, 
            {-2, 1}
        }
    };

    std::array<std::array<int, 2>, 4> bishop_moves = { // The four direction a bishop can move
        {
            {-1, -1}, 
            {-1, 1}, 
            {1, -1}, 
            {1, 1}, 
        }
    };

    std::array<std::array<int, 2>, 4> rook_moves = { // The four direction a rook can move
        {
            {-1, 0}, 
            {1, 0}, 
            {0, -1}, 
            {0, 1}, 
        }
    };

    std::array<std::array<int, 2>, 8> king_moves = { // The four direction a rook can move
        {
            {-1, -1}, 
            {-1, 0}, 
            {-1, 1}, 
            {0, 1}, 
            {1, 1},
            {1, 0},
            {1, -1},
            {0, -1}
        }
    };
};

#endif // BOARD_H