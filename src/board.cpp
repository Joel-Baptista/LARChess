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

void Board::show(){
    char c;
    // Utilize only the fen's board part
    std::string board_fen = fen.substr(0, fen.find_first_of(' ')); 

    for(int i=0; i<board_fen.length(); i++){       
        c = board_fen[i];
        if (std::isalpha(c)){
            std::cout  << " " << c << " ";
        }else if(std::isdigit(c)){   
            for (int j=0; j<int(c) - 48;j++){
                std::cout << " . "; 
            }
        }else if(c=='/'){
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
};

void Board::reset(){
    fen = start_fen;
    update_board_from_fen(fen);
}

void Board::make_move(Move move){
    char player = update_turn_from_fen();

    int from_row = (move.getFrom()[1] - '0');
    int from_col = move.getFrom()[0] - 'a';

    int to_row = (move.getTo()[1] - '0');
    int to_col = move.getTo()[0] - 'a';

    board[to_row][to_col] = board[from_row][from_col];
    board[from_row][from_col] = EMPTY;
    
    // Update board agnostic information of fen
    int idx1 = find_nth(fen, ' ', 3); // On passant space 
    std::cout << player << std::endl;
    std::cout << from_row << std::endl;
    std::cout << to_row << std::endl;
    std::cout << move.getFrom()[0] << "3" << std::endl;
    if (player == 0 && from_row == 2 && to_row == 4){ // White pawn double move
        std::string s;  s += move.getFrom()[0]; s += '3';
        fen.insert(idx1 + 1,  s);
    }
    else if (player == 1 && from_row == 7 && to_row == 5){ // Black pawn double move
        std::string s;  s += move.getFrom()[0]; s += '6';
        fen.insert(idx1 + 1, s);
    }else{
        fen[idx1 + 1] = '-';
    }


    fen[fen.find_first_of(' ') + 1] = (fen[fen.find_first_of(' ') + 1] == 'w') ? 'b' : 'w'; // Change player
    
    // Update board part of fen
    update_fen_from_board(board); 
}



