#include "board.h"
#include "utils.h"
#include <iostream>
#include <algorithm>
#include "fen.cpp"


Board::Board(){
    board = {0};
    fen = start_fen;
    turn_player = update_turn_from_fen();
    update_board_from_fen(fen);
    update_castling_rights_from_fen();
    update_en_passant_from_fen();
    update_halfmove_clock_from_fen();
    update_fullmove_number_from_fen();
}

Board::Board(std::string fen_init){
    board = {0};
    fen = fen_init;
    turn_player = update_turn_from_fen();
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
    if (turn_player == 1 && from_row == 6 && to_row == 4){ // White pawn double move
        std::string s;  s += move.getFrom()[0]; s += '3';
        en_passant = s;
    }
    else if (turn_player == -1 && from_row == 1 && to_row == 3){ // Black pawn double move
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
    if (turn_player == -1){
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
    turn_player = (turn_player == 1) ? -1 : 1;

}

std::unordered_map<std::string, std::vector<Move>> Board::get_all_legal_moves(){
    std::unordered_map<std::string, std::vector<Move>> all_legal_moves = {};

    for (int i=0; i<8; i++){
        for (int j=0; j<8; j++){
            std::string square = std::string(1, (char)('a' + j)) + (char)('8' - i);
            all_legal_moves[square] = get_legal_moves(square);
        }
    }

    return all_legal_moves;
}

std::vector<Move> Board::get_legal_moves(std::string square){


    std::vector<Move> square_legal_moves = {};
    int from_row = 8 - (square[1] - '0');
    int from_col = square[0] - 'a';
    int piece = board[from_row][from_col];

    if (board[from_row][from_col] == EMPTY){
        std::cout << "No piece in square\n";
        return square_legal_moves;
    }

    if (legal_moves_calculated){
        std::cout << "Legal moves already calculated\n";
        return legal_moves[square];
    }

    if (turn_player == 1 && board[from_row][from_col] < 0){
        std::cout << "Not your piece " << turn_player << " " << board[from_row][from_col] << "\n";
        return square_legal_moves;
    } else if (turn_player == -1 && board[from_row][from_col] > 0){
        std::cout << "Not your piece " << turn_player << " " << board[from_row][from_col] << "\n";
        return square_legal_moves;
    }

    switch (abs(piece))
    {
    case W_PAWN:    
        square_legal_moves = get_pawn_moves(from_row, from_col);
        break;
    case W_KNIGHT:
        square_legal_moves = get_knight_moves(from_row, from_col);
        break;
    case W_BISHOP:
        square_legal_moves = get_bishop_moves(from_row, from_col);
        break;
    case W_ROOK:
        square_legal_moves = get_rook_moves(from_row, from_col);
        break;
    case W_QUEEN:
        square_legal_moves = get_queen_moves(from_row, from_col);
        break;
    default:
        break;
    }

    // std::cout << "Legal moves for " << square_legal_moves.size() << "\n";

    for (int i=0; i<square_legal_moves.size(); i++){
        std::cout << square_legal_moves[i].getFrom() << square_legal_moves[i].getTo() << square_legal_moves[i].getPromotion() << "\n";
    }

    
    return square_legal_moves;
}


std::vector<Move> Board::get_pawn_moves(int row, int col){
    std::vector<Move> moves = {};
    

    // Check if first move
    if (((row == 6 && turn_player == 1) || (row == 1 && turn_player == -1)) &&
         board[row - turn_player][col] == EMPTY && 
         board[row - 2 * turn_player][col] == EMPTY){ // If the pawn is in the starting rank for white or black and the first two squares are empty

        std::string s1; s1 += (char)('a' + col); s1 += (char)('8' - (row));
        std::string s2; s2 += (char)('a' + col); s2 += (char)('8' - (row - turn_player));

        moves.push_back(Move(s1, s2));
        s2[1] = (char)(s2[1] + turn_player);
        moves.push_back(Move(s1, s2));

    } else if (board[row - turn_player][col] == EMPTY){ // If it not the first move and the square in front of the pawn is empty
        std::string s1; s1 += (char)('a' + col); s1 += (char)('8' - (row));
        std::string s2; s2 += (char)('a' + col); s2 += (char)('8' - (row - turn_player));

        if (row == 1 || row == 6){ // If the pawn is in the last rank. The check is agnostic to the turn player because we already checked if the pawn is in the starting rank
            moves.push_back(Move(s1, s2, 'q'));
            moves.push_back(Move(s1, s2, 'r'));
            moves.push_back(Move(s1, s2, 'n'));
            moves.push_back(Move(s1, s2, 'b'));
        }else{
            moves.push_back(Move(s1, s2));
        }

    }

    // Check if there is a piece to capture
    
    std::array<int, 2> capture_cols = {1, -1};

    for (int i=0; i<2; i++){
        if (col + capture_cols[i] >= 0 && col + capture_cols[i] < 8){
            std::string capture_square = std::string(1, (char)('a' + col + capture_cols[i])) + (char)('8' - (row - turn_player));
            if (board[row - turn_player][col + capture_cols[i]] * turn_player < 0
                || en_passant == capture_square){ 
                std::string s1; s1 += (char)('a' + col); s1 += (char)('8' - (row));


                if (row - turn_player == 0 || row - turn_player == 7){ // If the pawn is in the last rank. The check is agnostic to the turn player because we already checked if the pawn is in the starting rank
                    moves.push_back(Move(s1, capture_square, 'q'));
                    moves.push_back(Move(s1, capture_square, 'r'));
                    moves.push_back(Move(s1, capture_square, 'n'));
                    moves.push_back(Move(s1, capture_square, 'b'));
                }else{
                    moves.push_back(Move(s1, capture_square));
                }
            }
        }
    }

    //TODO: Check pins

    return moves;

}

std::vector<Move> Board::get_knight_moves(int row, int col){
    std::vector<Move> moves = {};

    for (int i=0; i<8; i++){
        int new_row = row + knight_moves[i][0];
        int new_col = col + knight_moves[i][1];

        if (new_row >= 0 && new_row < 8 && new_col >= 0 && new_col < 8){
            if (board[new_row][new_col] * turn_player <= 0){ // Capture or empty square
                std::string s1; s1 += (char)('a' + col); s1 += (char)('8' - (row));
                std::string s2; s2 += (char)('a' + new_col); s2 += (char)('8' - (new_row));
                moves.push_back(Move(s1, s2));
            }
        }
    }
    // TODO: Add pins
    return moves;
}
std::vector<Move> Board::get_bishop_moves(int row, int col){
    std::vector<Move> moves = {};

    for (int i=0; i<4; i++){
        for (int j=1; j<8; j++){
            int new_row = row + j * bishop_moves[i][0];
            int new_col = col + j * bishop_moves[i][1];

            if (new_row >= 0 && new_row < 8 && new_col >= 0 && new_col < 8){
                if (board[new_row][new_col] * turn_player <= 0){ // Capture or empty square
                    std::string s1; s1 += (char)('a' + col); s1 += (char)('8' - (row));
                    std::string s2; s2 += (char)('a' + new_col); s2 += (char)('8' - (new_row));
                    moves.push_back(Move(s1, s2));
                }
            }
            if (board[new_row][new_col] != EMPTY){
                break;
            }


        }
    }

    // TODO: Add pins

    return moves;
}
std::vector<Move> Board::get_rook_moves(int row, int col){
    std::vector<Move> moves = {};

    for (int i=0; i<4; i++){
        for (int j=1; j<8; j++){
            int new_row = row + j * rook_moves[i][0];
            int new_col = col + j * rook_moves[i][1];

            if (new_row >= 0 && new_row < 8 && new_col >= 0 && new_col < 8){
                if (board[new_row][new_col] * turn_player <= 0){ // Capture or empty square
                    std::string s1; s1 += (char)('a' + col); s1 += (char)('8' - (row));
                    std::string s2; s2 += (char)('a' + new_col); s2 += (char)('8' - (new_row));
                    moves.push_back(Move(s1, s2));
                }
            }
            if (board[new_row][new_col] != EMPTY){
                break;
            }
        }
    }

    // TODO: Add pins

    return moves;

}
std::vector<Move> Board::get_queen_moves(int row, int col){
    std::vector<Move> moves = {};

    std::vector<Move> rook_moves = get_rook_moves(row, col);
    std::vector<Move> bishop_moves = get_bishop_moves(row, col);


    moves.insert(moves.end(), rook_moves.begin(), rook_moves.end());
    moves.insert(moves.end(), bishop_moves.begin(), bishop_moves.end());

    // TODO: Add pins

    return moves;

}
std::vector<Move> Board::get_king_moves(int row, int col){
    std::vector<Move> moves = {};

    for (int i=0; i<8; i++){
        int new_row = row + king_moves[i][0];
        int new_col = col + king_moves[i][1];

        bool empty_or_capture = board[new_row][new_col] * turn_player <= 0;
        bool seen_by_knight = false;
        bool seen_diagonal = false;
        bool seen_straight = false;

        for (int j=0; j<8; j++){ // Check if there is a knight seeing that square
            int knight_row = new_row + knight_moves[j][0];
            int knight_col = new_col + knight_moves[j][1];
            if (knight_row >= 0 && knight_row < 8 && knight_col >= 0 && knight_col < 8){
                if (board[knight_row][knight_col] * turn_player == -2){
                    seen_by_knight = true; 
                    break;
                }
                if (board[knight_row][knight_col] != EMPTY){
                        break;
                }
            }
        }

        for (int j=0; j<4; j++){ // Check if there is a bishop or queen or pawn seeing that square (NOTE: The pawn is not a valid piece to see the king sometimes)
            for (int k=1; k<8; k++){
                int bishop_row = new_row + k * bishop_moves[j][0];
                int bishop_col = new_col + k * bishop_moves[j][1];
                if (bishop_row >= 0 && bishop_row < 8 && bishop_col >= 0 && bishop_col < 8){
                    if (board[bishop_row][bishop_col] * turn_player == -3 || board[bishop_row][bishop_col] * turn_player == -5 || board[bishop_row][bishop_col] * turn_player == -1){
                        seen_diagonal = true;
                        break;
                    }
                    if (board[bishop_row][bishop_col] != EMPTY){
                        break;
                    }
                }
            }
        }

        for (int j=0; j<4; j++){ // Check if there is a rook or queen seeing that square
            for (int k=1; k<8; k++){
                int rook_row = new_row + k * rook_moves[j][0];
                int rook_col = new_col + k * rook_moves[j][1];
                if (rook_row >= 0 && rook_row < 8 && rook_col >= 0 && rook_col < 8){
                    if (board[rook_row][rook_col] * turn_player == -4 || board[rook_row][rook_col] * turn_player == -5){
                        seen_straight = true;
                        break;
                    }
                    if (board[rook_row][rook_col] != EMPTY){
                        break;
                    }
                }
            }
        }

        if (new_row >= 0 && new_row < 8 && new_col >= 0 && new_col < 8){
            if (empty_or_capture && !seen_by_knight && !seen_diagonal && !seen_straight){ // Capture or empty square
                std::string s1; s1 += (char)('a' + col); s1 += (char)('8' - (row));
                std::string s2; s2 += (char)('a' + new_col); s2 += (char)('8' - (new_row));
                moves.push_back(Move(s1, s2));
            }
        }
    }

    return moves;


}

