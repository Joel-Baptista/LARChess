#include <iostream>
#include "board.h"
#include "move.h"

int main() {
    Board board;
    board.print();

    std::string from, to;
    while (true) {
        std::cout << "Enter move (e.g., e2 e4): ";
        std::cin >> from >> to;
        
        if (from == "quit" || to == "quit") break;
        
        Move move(from, to);
        if (board.makeMove(move)) {
            board.print();
        } else {
            std::cout << "Invalid move. Try again." << std::endl;
        }
    }

    return 0;
}