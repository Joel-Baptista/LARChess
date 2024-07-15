#ifndef BOARD_H
#define BOARD_H

#include <string>
#include <vector>
#include "move.h"

class Board {
public:
    Board();
    std::string start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::string fen;
    void show();
    char turn();
    bool castling_rights(char player, char side);

private:

};

#endif // BOARD_H