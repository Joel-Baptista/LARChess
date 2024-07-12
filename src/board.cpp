#include "board.h"
#include "utils.h"
#include <iostream>

Board::Board() {
    setupBoard();
}

void Board::setupBoard() {
    board = {
        {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
        {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
        {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}
    };
}

void Board::print() const {
    for (const auto& row : board) {
        for (char piece : row) {
            std::cout << piece << " ";
        }
        std::cout << std::endl;
    }
}

bool Board::makeMove(const Move& move) {
    if (!isMoveValid(move)) return false;

    int fromRow = 8 - (move.getFrom()[1] - '0');
    int fromCol = move.getFrom()[0] - 'a';
    int toRow = 8 - (move.getTo()[1] - '0');
    int toCol = move.getTo()[0] - 'a';

    board[toRow][toCol] = board[fromRow][fromCol];
    board[fromRow][fromCol] = ' ';
    return true;
}

bool Board::isMoveValid(const Move& move) const {
    int fromRow = 8 - (move.getFrom()[1] - '0');
    int fromCol = move.getFrom()[0] - 'a';
    int toRow = 8 - (move.getTo()[1] - '0');
    int toCol = move.getTo()[0] - 'a';

    if (!isWithinBounds(fromRow, fromCol) || !isWithinBounds(toRow, toCol)) {
        return false;
    }

    char piece = board[fromRow][fromCol];
    if (piece == ' ') return false;

    // Simple validation: move to an empty square or capture an opponent's piece
    char target = board[toRow][toCol];
    if (target == ' ' || (isupper(piece) != isupper(target))) {
        return true;
    }

    return false;
}

bool Board::isWithinBounds(int row, int col) const {
    return row >= 0 && row < 8 && col >= 0 && col < 8;
}
