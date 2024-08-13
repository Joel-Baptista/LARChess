#pragma once

#include "../../BBChessEngine/src/bit_board.h"

class Game
{
    public:
        Game();
        ~Game();
    

    
    private:


        BitBoard m_Board;

};

struct state
{
    U64 bitboards[12]; // 12 bitboards for each piece type (6 for white and 6 for black)
    U64 occupancies[3]; // occupancy bitboards for each color and all pieces
    int side; // side to move
    int en_passant_square = 64;
    int castle_rights; 
    int halfmove;
    int fullmove;
};