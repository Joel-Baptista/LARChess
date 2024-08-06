#include "board.h"
#include "utils.h"
#include <iostream>

void Board::show_from_fen(){
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


void Board::update_fen(){
    update_fen_from_board(board);
    fen += " ";
    fen += (turn_player == 1) ? "w" : "b";
    fen += " ";
    fen += (castling_rights[0]) ? "K" : "";
    fen += (castling_rights[1]) ? "Q" : "";
    fen += (castling_rights[2]) ? "k" : "";
    fen += (castling_rights[3]) ? "q" : "";
    fen += " ";
    fen += en_passant;
    fen += " ";
    fen += std::to_string(halfmove_clock);
    fen += " ";
    fen += std::to_string(fullmove_number);
}

void Board::update_castling_rights_from_fen(){
    // Second and third occurance of space delimit castling rights
    int idx1 = find_nth(fen, ' ', 2);
    int idx2 = find_nth(fen, ' ', 3);

    std::string castling_fen = fen.substr(idx1 +1, idx2 - idx1 - 1); 
    castling_rights = {false, false, false, false};
    
    for(int i=0; i < castling_fen.length(); i++){
        switch (castling_fen[i])
        {
        case 'K':
            castling_rights[0] = true;
            break;
        case 'Q':
            castling_rights[1] = true;
            break;
        case 'k':
            castling_rights[2] = true;
            break;  
        case 'q':   
            castling_rights[3] = true;
        default:
            break;
        } 
    }
}

void Board::update_board_from_fen(const std::string& fen){

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

void Board::update_fen_from_board(const std::array<std::array<int, 8>, 8> board){
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
    fen = board_fen;
}

int Board::update_turn_from_fen(){
    char turn = fen[fen.find_first_of(' ') + 1];
    turn_player = (turn == 'w') ? 1 : -1;
    return turn_player;
}

void Board::update_en_passant_from_fen(){
    // Fifth occurance of space delimits en passant square
    int idxS = find_nth(fen, ' ', 3);
    int idxE = find_nth(fen, ' ', 4);
    en_passant = fen.substr(idxS + 1, idxE - idxS - 1);
}

void Board::update_halfmove_clock_from_fen(){
    // Fourth occurance of space delimits halfmove clock
    int idxS = find_nth(fen, ' ', 4);
    int idxE = find_nth(fen, ' ', 5);
    halfmove_clock = std::stoi(fen.substr(idxS + 1, idxE - idxS - 1));
}
void Board::update_fullmove_number_from_fen(){
    // Sixth occurance of space delimits fullmove number
    int idxS = find_nth(fen, ' ', 5);
    int idxE = fen.length();
    fullmove_number = std::stoi(fen.substr(idxS + 1, idxE - idxS - 1));
}

std::string Board::get_fen(){
    update_fen();
    return fen;

}
void Board::set_fen(std::string new_fen){
    fen = new_fen;
    update_board_from_fen(fen);
    update_turn_from_fen();
    update_castling_rights_from_fen();
    update_en_passant_from_fen();
    update_halfmove_clock_from_fen();
    update_fullmove_number_from_fen();
}