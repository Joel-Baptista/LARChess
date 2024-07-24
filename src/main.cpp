#include <iostream>
#include "board.h"
#include "move.h"


int main() {
    Board board;

    std::vector<std::string> moves = {"e2e4", "e7e5", "g1f3", "b8c6", "f1c4", "g8f6", "d2d3", "d7d6", "c1e3", "c8d7", "e1g1", "e8g8"};    

    for (int i=0; i<moves.size(); i++){
        Move move(moves[i].substr(0,2), moves[i].substr(2,2));
        board.make_move(move);
    
        // for (int i=0; i<8; i++){
        //     for (int j=0; j<8; j++){
        //         std::cout << board.board[i][j] << " ";
        //     }
        //     std::cout << "\n";
        // }
        board.show();

        std::cout << "player turn: " << board.turn_player << "\n";
        std::cout << "en passant: " << board.en_passant << "\n";
        std::cout << "half move: " << board.halfmove_clock << "\n";
        std::cout << "fullmove: " << board.fullmove_number << "\n";
        std::cout << "castling rights: " << board.castling_rights[0] << board.castling_rights[1] << board.castling_rights[2] << board.castling_rights[3] << "\n";
        

        std::cout << board.get_fen() << "\n";
        
    }
    
    // board.show();

    return 0;
}