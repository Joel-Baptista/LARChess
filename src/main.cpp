#include <iostream>
#include "board.h"
#include "move.h"

int main() {
    Board board;
    std::cout << board.fen << "\n";

    Move move("e2", "e4");
    board.make_move(move);

    
    for (int i=0; i<8; i++){
        for (int j=0; j<8; j++){
            std::cout << board.board[i][j] << " ";
        }
        std::cout << "\n";
    }

    std::cout << board.fen << "\n";
    board.show();

    
    std::cout << board.castling_rights('w', 'k') << std::endl;

    return 0;
}