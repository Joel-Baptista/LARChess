#include <iostream>
#include "board.h"
#include "move.h"
#include "utils.h"


int main() {
    Board board;

    std::vector<std::string> moves = {"e2e4", "e7e5", "g1f3", "b8c6", "f1c4", "g8f6", "d2d3", "d7d6", "c1e3", "c8d7", "e1g1", "e8g8"};    
    // std::vector<std::string> states = {
    //     "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1", // Check pawn capture
    //     "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 1", // Check en passant
    //     "r1bqkbnr/pPp1p1pp/8/3p1p2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 1", // Check promotion
        // "r1bqkbnr/p1p1p1pp/8/5p2/2Pp4/4P3/PP1P1PPP/RNBQKBNR b KQkq c3 0 1", // Check capture and en passant black
        // "r1bqkbnr/p1p1p1pp/8/5p2/2P5/4P3/PPpP1PPP/RN1QKBNR b KQkq c3 0 1", // check promotion black
        // "r1bqkbnr/p1p1p1pp/8/3N1p2/2P5/4P3/PPpP1PPP/R2QKBNR w KQkq c3 0 1", // check knight move
        // "r1bqkbnr/p1p1p1pp/8/1N3p2/2P5/4P3/PPpP1PPP/R2QKBNR w KQkq c3 0 1", // check knight move
        // "r1bqkbnr/p1p1p1pp/8/1N3p2/2P5/3BP3/PPpP1PPP/R2QK1NR w KQkq - 0 1", // check bishop move
        // "r1b1kbnr/p1pqp1pp/8/1N3p2/2PR4/3BP3/PPpP1PPP/3QK1NR w Kkq - 0 1", // check rook move
        // "r1b1kbnr/p1pqp1pp/8/1N3p2/2PR2Q1/3BP3/PPpP1PPP/4K1NR w Kkq - 0 1", // check queen move
        // "r3k3/ppp2bpp/2np4/4qp1B/BbPP4/2Qn4/PPP1RPPP/1N2K1Nr b q - 0 1", // check movement of pinned pieces
        // "r3k3/ppp2bpp/2np4/4qp1B/BbPP4/2Qn4/PPP1RPPP/1N2K1Nr b q - 0 1", // check movement of pinned pieces
        // "r3k3/ppp2bpp/2np4/4qp1B/BbPP4/2Qn4/PPP1RPPP/1N2K1Nr b q - 0 1", // check movement of pinned pieces
        // "rn1qkbnr/pppppppp/8/8/8/6b1/PPPPPPPP/RNBQKBNR w KQkq - 0 1", // check movement of pinned pawn
        // "rn2kbnr/pppppppp/8/4q3/8/3b4/PPPPPPPP/RNBQKBNR w KQkq - 0 1" // check movement of pinned pawn
    // };
    // std::vector<std::string> squares = {
    //     "e4",
    //     "e5",
    //     "b7",
        // "d4",
        // "c2",
        // "d5",
        // "b5",
        // "d3",
        // "d4",
        // "g4",
        // "e5",
        // "c6",
        // "f7",
        // "f2",
        // "e2",
    // };

    // for (int i=0; i<states.size(); i++){
    //     std::cout << "State " << i << "\n";
    //     board.set_fen(states[i]);
    //     board.show();

    //     Board::Pin pin = board.is_pinned(squares[i]);
    //     if (pin.is_pinned){
    //         std::cout << "Pinned piece at " << squares[i] << " to " << pin.enemy_square << "\n";
    //     }
    //     // std::vector<Move> legal_moves = board.get_legal_moves(squares[i]);
    //     board.get_all_legal_moves();
    //     std::cout << "---------------------------------------------\n";
    // }

    // board.set_fen("rBb1kb1r/ppp3pp/2np2n1/4pp1Q/B1PP4/7q/PPP1RPPP/RN2K1N1 b Qkq - 0 1"); // to check pins
    // board.set_fen("rB2k3/ppp3pp/2np2n1/4qp2/BbPP4/2Q5/PPP1RPPP/bN2K1Nr w q - 0 1"); // to check pins black
    // std::vector<std::string> pinned_squares = {"g6", "e5", "c6", "c8"};

    // std::cout << board.turn_player << "\n";

    // int count = 0;

    // for (int i=0; i<8; i++){
    //     for (int j=0; j<8; j++){
    //         Board::Pin pin = board.is_pinned(i, j);
    //         if (pin.is_pinned){
    //             std::string s = coordinates_to_square(i, j);
    //             std::cout << "Pinned piece at " << s << " to " << pin.enemy_square << "\n";
    //             count++;
    //             std::cout << "---------------------------------------------\n";
    //         }
    //     }
    // }

    // int idx = 0;
    // for (int i=0; i<1000000; i++){
    //     std::cout << "State " << i << "\n";
    //     board.set_fen(states[idx]);
    //     // board.show();
    //     std::unordered_map<std::string, std::vector<Move>> all_legal_moves = board.get_all_legal_moves();

    //     idx++;
    //     if (idx >= states.size()){
    //         idx = 0;
    //     }
    // }

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

    // return 0;
}