#ifndef BOARD_H
#define BOARD_H

#include <string>
#include <vector>
#include "move.h"

class Board {
public:
    Board();
    void print() const;
    bool makeMove(const Move& move);
    bool isMoveValid(const Move& move) const;

private:
    std::vector<std::vector<char>> board;
    void setupBoard();
    bool isWithinBounds(int row, int col) const;
};

#endif // BOARD_H