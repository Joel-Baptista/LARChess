#include <iostream>
#include "board.h"
#include "move.h"


int main() {
    Board board;

    std::vector<std::string> moves = {"e2e4", "e7e5", "g1f3", "b8c6", "f1c4", "g8f6", "d2d3", "d7d6", "c1e3", "c8d7", "e1g1", "e8g8"};    
    std::array<std::string, 10> states = {
        "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1", // Check pawn capture
        "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 1", // Check en passant
        "r1bqkbnr/pPp1p1pp/8/3p1p2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 1", // Check promotion
        "r1bqkbnr/p1p1p1pp/8/5p2/2Pp4/4P3/PP1P1PPP/RNBQKBNR b KQkq c3 0 1", // Check capture and en passant black
        "r1bqkbnr/p1p1p1pp/8/5p2/2P5/4P3/PPpP1PPP/RN1QKBNR b KQkq c3 0 1", // check promotion black
        "r1bqkbnr/p1p1p1pp/8/3N1p2/2P5/4P3/PPpP1PPP/R2QKBNR w KQkq c3 0 1", // check knight move
        "r1bqkbnr/p1p1p1pp/8/1N3p2/2P5/4P3/PPpP1PPP/R2QKBNR w KQkq c3 0 1", // check knight move
        "r1bqkbnr/p1p1p1pp/8/1N3p2/2P5/3BP3/PPpP1PPP/R2QK1NR w KQkq - 0 1", // check bishop move
        "r1b1kbnr/p1pqp1pp/8/1N3p2/2PR4/3BP3/PPpP1PPP/3QK1NR w Kkq - 0 1", // check rook move
        "r1b1kbnr/p1pqp1pp/8/1N3p2/2PR2Q1/3BP3/PPpP1PPP/4K1NR w Kkq - 0 1", // check queen move
    };
    std::array<std::string, 10> squares = {
        "e4",
        "e5",
        "b7",
        "d4",
        "c2",
        "d5",
        "b5",
        "d3",
        "d4",
        "g4",
    };

    for (int i=0; i<states.size(); i++){
        std::cout << "State " << i << "\n";
        board.set_fen(states[i]);
        board.show();
        std::vector<Move> legal_moves = board.get_legal_moves(squares[i]);
    }



    // for (int i=0; i<1000000; i++){
    //     std::cout << "State " << i << "\n";
    //     board.set_fen(board.start_fen);
    //     // board.show();
    //     std::unordered_map<std::string, std::vector<Move>> all_legal_moves = board.get_all_legal_moves();
    // }

    // for (int i=0; i<moves.size(); i++){
    //     Move move(moves[i].substr(0,2), moves[i].substr(2,2));
        // board.make_move(move);
    
    //     // for (int i=0; i<8; i++){
    //     //     for (int j=0; j<8; j++){
    //     //         std::cout << board.board[i][j] << " ";
    //     //     }
    //     //     std::cout << "\n";
    //     // }
    //     board.show();

    //     std::cout << "player turn: " << board.turn_player << "\n";
    //     std::cout << "en passant: " << board.en_passant << "\n";
    //     std::cout << "half move: " << board.halfmove_clock << "\n";
    //     std::cout << "fullmove: " << board.fullmove_number << "\n";
    //     std::cout << "castling rights: " << board.castling_rights[0] << board.castling_rights[1] << board.castling_rights[2] << board.castling_rights[3] << "\n";
        

    //     std::cout << board.get_fen() << "\n";
        
    // }
    
    // board.show();

    return 0;
}