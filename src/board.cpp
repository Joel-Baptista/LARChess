#include "board.h"
#include "utils.h"
#include <iostream>
#include <algorithm>


Board::Board(){
    board = {0};
    fen = start_fen;
    board_from_fen(fen);
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
    board_from_fen(fen);
}

void Board::make_move(Move move){
    char player = this->turn();

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
    if (player == 'w' && from_row == 2 && to_row == 4){
        std::string s;  s += move.getFrom()[0]; s += '3';
        fen.insert(idx1 + 1,  s);
    }
    else if (player == 'b' && from_row == 7 && to_row == 5){
        std::string s;  s += move.getFrom()[0]; s += '6';
        fen.insert(idx1 + 1, s);
    }else{
        fen[idx1 + 1] = '-';
    }


    fen[fen.find_first_of(' ') + 1] = (fen[fen.find_first_of(' ') + 1] == 'w') ? 'b' : 'w'; // Change player

    
    // Update board part of fen
    fen_from_board(board); 
}

void Board::board_from_fen(const std::string& fen){

    std::string board_fen = fen.substr(0, fen.find_first_of(' '));

    int count = 0;
    int row = 0;
    int col = 0;
    for (int i=0; i<board_fen.length(); i++){
        char c = fen[i];

        if (std::isalpha(c)){
            switch (c){
                case 'P':
                    board[row][col] = W_PAWN;
                    break;
                case 'N':
                    board[row][col] = W_KNIGHT;
                    break;
                case 'B':
                    board[row][col] = W_BISHOP;
                    break;
                case 'R':
                    board[row][col] = W_ROOK;
                    break;
                case 'Q':
                    board[row][col] = W_QUEEN;
                    break;
                case 'K':
                    board[row][col] = W_KING;
                    break;
                case 'p':
                    board[row][col] = B_PAWN;
                    break;
                case 'n':
                    board[row][col] = B_KNIGHT;
                    break;
                case 'b':
                    board[row][col] = B_BISHOP;
                    break;
                case 'r':
                    board[row][col] = B_ROOK;
                    break;
                case 'q':
                    board[row][col] = B_QUEEN;
                    break;
                case 'k':
                    board[row][col] = B_KING;
                    break;
            }
            col++;
        }else if(std::isdigit(c)){
            for (int j=0; j<int(c) - 48;j++){
                board[row][col] = EMPTY;
                col++;
            }
        }else if(c=='/'){
            row++;
            col = 0;
        }
    }
}

void Board::fen_from_board(const std::array<std::array<int, 8>, 8> board){
    std::string board_fen = "";
    int empty_count = 0;
    for (int i=0; i<8; i++){
        for (int j=0; j<8; j++){
            if (board[i][j] == EMPTY){
                empty_count++;
            }else{
                if (empty_count > 0){
                    board_fen += std::to_string(empty_count);
                    empty_count = 0;
                }
                switch (board[i][j]){
                    case W_PAWN:
                        board_fen += "P";
                        break;
                    case W_KNIGHT:
                        board_fen += "N";
                        break;
                    case W_BISHOP:
                        board_fen += "B";
                        break;
                    case W_ROOK:
                        board_fen += "R";
                        break;
                    case W_QUEEN:
                        board_fen += "Q";
                        break;
                    case W_KING:
                        board_fen += "K";
                        break;
                    case B_PAWN:
                        board_fen += "p";
                        break;
                    case B_KNIGHT:
                        board_fen += "n";
                        break;
                    case B_BISHOP:
                        board_fen += "b";
                        break;
                    case B_ROOK:
                        board_fen += "r";
                        break;
                    case B_QUEEN:
                        board_fen += "q";
                        break;
                    case B_KING:
                        board_fen += "k";
                        break;
                }
            }
        }
        if (empty_count > 0){
            board_fen += std::to_string(empty_count);
            empty_count = 0;
        }
        if (i < 7){
            board_fen += "/";
        }
    }
    fen = board_fen + fen.substr(fen.find_first_of(' '));
    std::cout << fen << std::endl;

}


char Board::turn(){
    char player = fen[fen.find_first_of(' ') + 1];
    return player;

}

bool Board::castling_rights(char player, char side){
    // Second and third occurance of space delimit castling rights
    int idx1 = find_nth(fen, ' ', 2);
    int idx2 = find_nth(fen, ' ', 3);

    std::string castling_fen = fen.substr(idx1 +1, idx2 - idx1 - 1); 
    
    for(int i=0; i < castling_fen.length(); i++){
        if (tolower(player) == 'w' && toupper(side) == castling_fen[i]){
            return true;
        }else if(tolower(player) == 'b' && tolower(side) == castling_fen[i]){
            return true;
        }
    }
    return false;
}