#include "board.h"
#include "utils.h"
#include <iostream>
#include <algorithm>
#include "fen.cpp"


Board::Board(){
    board = {0};
    fen = start_fen;
    update_board_from_fen(fen);
    update_castling_rights_from_fen();
    update_en_passant_from_fen();
    update_halfmove_clock_from_fen();
    update_fullmove_number_from_fen();
}

Board::Board(std::string fen_init){
    board = {0};
    fen = fen_init;
    update_board_from_fen(fen);
    update_castling_rights_from_fen();
    update_en_passant_from_fen();
    update_halfmove_clock_from_fen();
    update_fullmove_number_from_fen();
}

void Board::show(){
    for (int i=0; i<8; i++){
        for (int j=0; j<8; j++){

            char c;
            switch (abs(board[i][j]))
            {
            case W_PAWN:
                c = 'P';
                break;
            case W_KNIGHT:
                c = 'N';
                break;
            case W_BISHOP:
                c = 'B';
                break;
            case W_ROOK:    
                c = 'R';
                break;
            case W_QUEEN:   
                c = 'Q';
                break;
            case W_KING:    
                c = 'K';
                break;
            default:
                c = '.';
                break;
            }
            
            if (board[i][j] < 0){
                c = tolower(c);
            }
            
            std::cout << c << " ";
        }
        std::cout << "\n";
    }
}

void Board::reset(){
    fen = start_fen;
    update_board_from_fen(fen);
}

void Board::make_move(Move move){
    // Note: board rows are inverted compared to the normal matrix representation

    int from_row = 8 - (move.getFrom()[1] - '0');
    int from_col = move.getFrom()[0] - 'a';

    int to_row = 8 - (move.getTo()[1] - '0');
    int to_col = move.getTo()[0] - 'a';

    int piece = board[from_row][from_col];
    int target = board[to_row][to_col];


    board[to_row][to_col] = board[from_row][from_col];
    board[from_row][from_col] = EMPTY;


    // Check unpassant
    if (turn_player == 0 && from_row == 6 && to_row == 4){ // White pawn double move
        std::string s;  s += move.getFrom()[0]; s += '3';
        en_passant = s;
    }
    else if (turn_player == 1 && from_row == 1 && to_row == 3){ // Black pawn double move
        std::string s;  s += move.getFrom()[0]; s += '6';
        en_passant = s;
    }else{
        en_passant = '-';
    }

    // Update Halfmove clock
    if (target != EMPTY ||piece == W_PAWN || piece == B_PAWN){
        halfmove_clock = 0;
    }else{
        halfmove_clock++;
    }

    // Update Fullmove number
    if (turn_player == 1){
        fullmove_number++;
    }

    // Update castling rights
    if (piece == W_KING){
        castling_rights[0] = false;
        castling_rights[1] = false;
    }else if (piece == B_KING){
        castling_rights[2] = false;
        castling_rights[3] = false;
    }else if (move.getFrom() == "a1"){
        castling_rights[1] = false;
    }else if (move.getFrom() == "h1"){
        castling_rights[0] = false;
    }else if (move.getFrom() == "a8"){
        castling_rights[3] = false;
    }else if (move.getFrom() == "h8"){
        castling_rights[2] = false;
    }

    // Update turn player
    turn_player = (turn_player == 0) ? 1 : 0;

}



