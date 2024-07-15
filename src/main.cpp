#include <iostream>
#include "board.h"
#include "move.h"

int main() {
    Board board;
    std::cout << board.fen << "\n";
    std::cout << board.turn() << "\n";
    board.show();
    
    std::cout << board.castling_rights('w', 'k') << std::endl;

    return 0;
}