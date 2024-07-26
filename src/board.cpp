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
    
    std::vector<Move> legal_moves = get_legal_moves(move.getFrom());

    bool is_legal = false;

    for (int i=0; i<legal_moves.size(); i++){
        if (legal_moves[i].getFrom() == move.getFrom() && legal_moves[i].getTo() == move.getTo()){
            is_legal = true;
            break;
        }
    }

    if (!is_legal){return;} // If the move is not legal, do nothing

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
void Board::get_all_legal_moves(){
    legal_moves = {};

    for (int i=0; i<8; i++){
        for (int j=0; j<8; j++){
            std::string square = std::string(1, (char)('a' + j)) + (char)('8' - i);
            legal_moves[square] = get_legal_moves(square);
        }
    }
}
std::vector<Move> Board::get_legal_moves(std::string square){   

    if (legal_moves[square].size() != 0){ // If the legal moves have already been calculated
        return legal_moves[square];
    }

    std::vector<Move> square_legal_moves = {};

    Square_Coordinates from_sc = square_to_coordinates(square);

    int piece = board[from_sc.row][from_sc.col];

    if (board[from_sc.row][from_sc.col] == EMPTY){
        // std::cout << "No piece in square\n";
        return square_legal_moves;
    }

    // if (legal_moves_calculated){
    //     // std::cout << "Legal moves already calculated\n";
    //     return legal_moves[square];
    // }

    if (turn_player == 1 && board[from_sc.row][from_sc.col] < 0){
        // std::cout << "Not your piece " << turn_player << " " << board[from_row][from_col] << "\n";
        return square_legal_moves;
    } else if (turn_player == -1 && board[from_sc.row][from_sc.col] > 0){
        // std::cout << "Not your piece " << turn_player << " " << board[from_row][from_col] << "\n";
        return square_legal_moves;
    }

    switch (abs(piece))
    {
    case W_PAWN:    
        square_legal_moves = get_pawn_moves(from_sc.row, from_sc.col);
        break;
    case W_KNIGHT:
        square_legal_moves = get_knight_moves(from_sc.row, from_sc.col);
        break;
    case W_BISHOP:
        square_legal_moves = get_bishop_moves(from_sc.row, from_sc.col);
        break;
    case W_ROOK:
        square_legal_moves = get_rook_moves(from_sc.row, from_sc.col);
        break;
    case W_QUEEN:
        square_legal_moves = get_queen_moves(from_sc.row, from_sc.col);
        break;
    case W_KING:
        square_legal_moves = get_king_moves(from_sc.row, from_sc.col);
        break;
    default:
        break;
    }

    // std::cout << "Legal moves for " << square_legal_moves.size() << " for " << square << "\n";

    // for (int i=0; i<square_legal_moves.size(); i++){
    //     std::cout << square_legal_moves[i].getFrom() << square_legal_moves[i].getTo() << square_legal_moves[i].getPromotion() << "\n";
    // }

    
    return square_legal_moves;
}
std::vector<Move> Board::get_pawn_moves(int row, int col){
    std::vector<Move> moves = {};

    Pin pin = is_pinned(row, col);

    // Check if first move
    if (!pin.is_pinned || ((pin.is_pinned == pin.straight) && ((char)('a' + col) == pin.enemy_square[0]))){ // it's pinned atmost in the direction of the move
        if (((row == 6 && turn_player == 1) || (row == 1 && turn_player == -1)) &&
            board[row - turn_player][col] == EMPTY && 
            board[row - 2 * turn_player][col] == EMPTY){ // If the pawn is in the starting rank for white or black and the first two squares are empty

            std::string s1 = coordinates_to_square(row, col);
            std::string s2 = coordinates_to_square(row - turn_player, col);

            moves.push_back(Move(s1, s2));
            s2[1] = (char)(s2[1] + turn_player);
            moves.push_back(Move(s1, s2));

        } else if (board[row - turn_player][col] == EMPTY){ // If it not the first move and the square in front of the pawn is empty
            std::string s1 = coordinates_to_square(row, col);
            std::string s2 = coordinates_to_square(row - turn_player, col);
    
            if (row == 1 || row == 6){ // If the pawn is in the last rank. The check is agnostic to the turn player because we already checked if the pawn is in the starting rank
                moves.push_back(Move(s1, s2, 'q'));
                moves.push_back(Move(s1, s2, 'r'));
                moves.push_back(Move(s1, s2, 'n'));
                moves.push_back(Move(s1, s2, 'b'));
            }else{
                moves.push_back(Move(s1, s2));
            }
        }
    }

    // Check if there is a piece to capture
    
    std::array<int, 2> capture_cols = {1, -1};

    for (int i=0; i<2; i++){
        if (col + capture_cols[i] >= 0 && col + capture_cols[i] < 8){
            std::string capture_square = coordinates_to_square(row - turn_player, col + capture_cols[i]);
            if ((board[row - turn_player][col + capture_cols[i]] * turn_player < 0
                || en_passant == capture_square) &&
                (!pin.is_pinned || ((pin.is_pinned == pin.diagonal) && (capture_square == pin.enemy_square)))){ // it's pinned atmost in the direction of the move
                
                std::string s1 = coordinates_to_square(row, col);

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

    return moves;

}
std::vector<Move> Board::get_knight_moves(int row, int col){
    std::vector<Move> moves = {};

    Pin pin = is_pinned(row, col);

    if (pin.is_pinned){ // If the knight is pinned, it can't move ever
        return moves;
    }

    for (int i=0; i<8; i++){
        int new_row = row + knight_moves[i][0];
        int new_col = col + knight_moves[i][1];

        if (new_row >= 0 && new_row < 8 && new_col >= 0 && new_col < 8){
            if (board[new_row][new_col] * turn_player <= 0){ // Capture or empty square

                std::string s1 = coordinates_to_square(row, col);
                std::string s2 = coordinates_to_square(new_row, new_col);

                moves.push_back(Move(s1, s2));
            }
        }
    }

    return moves;
}
std::vector<Move> Board::get_bishop_moves(int row, int col){
    std::vector<Move> moves = {};

    Pin pin = is_pinned(row, col);

    // std::cout << "Is pinned " << pin.is_pinned << "\n";
    // std::cout << "Is diagonal " << pin.diagonal << "\n";
    // std::cout << "Is straight " << pin.straight << "\n";
    // std::cout << "Enemy square " << pin.enemy_square << "\n";
    // std::cout << "King square " << pin.king_square << "\n";

    if (pin.is_pinned && pin.straight){ // For bishop of queen, they cannot do diagonal moves if pinned in a straight line
        return moves;
    }
 
    int pin_diagonal = 0;
    
    if (pin.is_pinned && pin.diagonal){
        int direction_col = ((pin.enemy_square[0] - 'a') - col);
        int direction_row = (row - (8 - (pin.enemy_square[1] - '0')));

        pin_diagonal = direction_col / direction_row;
    
        // std::cout << "Direction " << direction_row << " " << direction_col << " " << pin_diagonal << "\n";
    }


    for (int i=0; i<4; i++){
        for (int j=1; j<8; j++){
            int new_row = row + j * bishop_moves[i][0];
            int new_col = col + j * bishop_moves[i][1];

            int diagonal = (new_col - col) / (row - new_row);

            if (new_row >= 0 && new_row < 8 && new_col >= 0 && new_col < 8){
                if (board[new_row][new_col] * turn_player <= 0 &&
                (!(pin.is_pinned == pin.diagonal) || diagonal == pin_diagonal)){ // Capture or empty square, or the piece is pinned in the direction of the move

                    std::string s1 = coordinates_to_square(row, col);
                    std::string s2 = coordinates_to_square(new_row, new_col);
                    moves.push_back(Move(s1, s2));
                }
            }
            if (board[new_row][new_col] != EMPTY){
                break;
            }


        }
    }

    return moves;
}
std::vector<Move> Board::get_rook_moves(int row, int col){
    std::vector<Move> moves = {};

    Pin pin = is_pinned(row, col);

    if (pin.is_pinned && pin.diagonal){ // For rook of queen, they cannot do straing line moves if pinned in a diagonal
        return moves;
    }
    
    int line_pin = 0; // 0 -> not pinned, 1 -> pinned in row, 2 -> pinned in col 

    if (pin.is_pinned && pin.straight){
        int direction_col = ((pin.enemy_square[0] - 'a') - col);
        int direction_row = (row - (8 - (pin.enemy_square[1] - '0')));

        line_pin = (direction_row == 0) ? 1 : 2;
    
        // std::cout << "Direction " << direction_row << " " << direction_col << " " << line_pin << "\n";
    }


    for (int i=0; i<4; i++){
        for (int j=1; j<8; j++){
            int new_row = row + j * rook_moves[i][0];
            int new_col = col + j * rook_moves[i][1];

            int line = (rook_moves[i][0] == 0) ? 1 : 2;

            if (new_row >= 0 && new_row < 8 && new_col >= 0 && new_col < 8){
                if (board[new_row][new_col] * turn_player <= 0 &&
                   (!(pin.is_pinned == pin.straight) || line == line_pin)){ // Capture or empty square, or the piece is pinned in the direction of the move
                    std::string s1 = coordinates_to_square(row, col);
                    std::string s2 = coordinates_to_square(new_row, new_col);
                    moves.push_back(Move(s1, s2));
                }
            }
            if (board[new_row][new_col] != EMPTY){
                break;
            }
        }
    }

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
                    // std::string s = coordinates_to_square(new_row, new_col);
                    // std::cout << "Seen by knight " << s << "\n";

                    break;
                }
            }
        }

        for (int j=0; j<4; j++){ // Check if there is a bishop or queen or pawn seeing that square (NOTE: The pawn is not a valid piece to see the king sometimes)
            for (int k=1; k<8; k++){
                int bishop_row = new_row + k * bishop_moves[j][0];
                int bishop_col = new_col + k * bishop_moves[j][1];
                if (bishop_row >= 0 && bishop_row < 8 && bishop_col >= 0 && bishop_col < 8){
                    if (board[bishop_row][bishop_col] * turn_player == -3 || board[bishop_row][bishop_col] * turn_player == -5 || (board[bishop_row][bishop_col] * turn_player == -1 && k == 1)){
                        seen_diagonal = true;
                        std::string s = coordinates_to_square(bishop_row, bishop_col);
                        // std::cout << "Seen by bishop, queen or pawn " << s << "\n";
                        // std::cout << "Piece " << (board[bishop_row][bishop_col] * turn_player) << "\n";
                        // std::cout << "Row " << bishop_row << " Col " << bishop_col << "\n";
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
                        // std::string s = coordinates_to_square(new_row, new_col);
                        // std::cout << "Seen by rook or queen " << s << "\n";
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
                std::string s1 = coordinates_to_square(row, col);
                std::string s2 = coordinates_to_square(new_row, new_col);
                moves.push_back(Move(s1, s2));
            }
        }
    }

    std::cout << "Legal moves for " << moves.size() << " for " << coordinates_to_square(row, col) << "\n";
    for (int i=0; i<moves.size(); i++){
        std::cout << moves[i].getFrom() << moves[i].getTo() << "\n";
    }

    return moves;


}
Board::Pin Board::is_pinned(int row, int col){
    
    if (board[row][col] == 0){ // Can't be pinned if there is no piece
        return {false, "", "", false, false};
    }

    int king_row = -1; int king_col = -1;

    for (int i=0; i<8; i++){ // Search for the king
        for (int j=0; j<8; j++){
            if (board[i][j] == W_KING * turn_player){
                king_row = i;
                king_col = j;
                break;
            }
        }
    }

    std::string king_square = coordinates_to_square(king_row, king_col);

    int direction_col = (king_col - col);
    int direction_row = (king_row - row);

    bool is_diagonal = (abs(direction_col) == abs(direction_row));
    bool is_straight = (direction_col == 0 || direction_row == 0);

    // Normalize directions
    if (direction_col != 0){ direction_col = direction_col / abs(direction_col);};
    if (direction_row != 0){ direction_row = direction_row / abs(direction_row);};

    // std::cout << "King at " << king_row << " " << king_col << "\n";
    // std::cout << "Piece at " << row << " " << col << "\n";
    // std::cout << "Direction " << direction_row << " " << direction_col << "\n";
    // std::cout << "Is diagonal " << is_diagonal << "\n";
    // std::cout << "Is straight " << is_straight << "\n"; 

    if (!is_diagonal && !is_straight){ // The king is not in the same line as the piece
        return {false, "", "", false, false};
    }

    for (int i=1; i<8; i++){ // Search for enemy piece in the oposite direction of the king
        int new_row = row - i * direction_row; 
        int new_col = col - i * direction_col;

        if (new_row >= 0 && new_row < 8 && new_col >= 0 && new_col < 8){
            if (is_straight && 
                (board[new_row][new_col] * turn_player == -4 ||
                board[new_row][new_col] * turn_player == -5)){ // Find enemy queen, rook in straight lines
                
                std::string s; s += (char)('a' + new_col); s += (char)('8' - new_row);
                return {true, s, king_square, false, true};
    
            } 
        
            if (is_diagonal && 
                (board[new_row][new_col] * turn_player == -3 || 
                board[new_row][new_col] * turn_player == -5)){ // Find enemy queen, rook in diagonals lines
                
                std::string s; s += (char)('a' + new_col); s += (char)('8' - new_row);
                return {true, s, king_square, true, false};
    
            }

            if (board[new_row][new_col] * turn_player > 0){ // Find friendly piece
                return {false, "", "", false, false};
            }
        }
    }

    return {false, "", "", false, false};

}
Board::Pin Board::is_pinned(std::string square){
    Square_Coordinates sc = square_to_coordinates(square);
    return is_pinned(sc.row, sc.col);
}
void Board::is_player_in_check(){
    player_in_check = false;

    int king_row = -1; int king_col = -1;

    for (int i=0; i<8; i++){ // Search for the king
        for (int j=0; j<8; j++){
            if (board[i][j] == W_KING * turn_player){
                king_row = i;
                king_col = j;
                break;
            }
        }
    }

    std::string king_square = coordinates_to_square(king_row, king_col);

    bool seen_by_knight = false;
    bool seen_diagonal = false;
    bool seen_straight = false;

    for (int j=0; j<8; j++){ // Check if there is a knight seeing that square
        int knight_row = king_row + knight_moves[j][0];
        int knight_col = king_col + knight_moves[j][1];
        if (knight_row >= 0 && knight_row < 8 && knight_col >= 0 && knight_col < 8){
            if (board[knight_row][knight_col] * turn_player == -2){
                 std::string s = coordinates_to_square(knight_row, knight_col);
                std::cout << "Check by knight " << s << "\n";
                seen_by_knight = true; 
                break;
            }
        }
    }

     for (int j=0; j<4; j++){ // Check if there is a bishop or queen or pawn seeing that square (NOTE: The pawn is not a valid piece to see the king sometimes)
            for (int k=1; k<8; k++){
                int bishop_row = king_row + k * bishop_moves[j][0];
                int bishop_col = king_col + k * bishop_moves[j][1];
                if (bishop_row >= 0 && bishop_row < 8 && bishop_col >= 0 && bishop_col < 8){
                    if (board[bishop_row][bishop_col] * turn_player == -3 || board[bishop_row][bishop_col] * turn_player == -5 || (board[bishop_row][bishop_col] * turn_player == -1 && k == 1)){
                        seen_diagonal = true;
                        std::string s = coordinates_to_square(bishop_row, bishop_col);
                        std::cout << "Check by bishop, queen or pawn " << s << "\n";
                        std::cout << "Piece " << (board[bishop_row][bishop_col] * turn_player) << "\n";
                        std::cout << "Row " << bishop_row << " Col " << bishop_col << "\n";
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
                int rook_row = king_col + k * rook_moves[j][0];
                int rook_col = king_col + k * rook_moves[j][1];
                if (rook_row >= 0 && rook_row < 8 && rook_col >= 0 && rook_col < 8){
                    if (board[rook_row][rook_col] * turn_player == -4 || board[rook_row][rook_col] * turn_player == -5){
                        seen_straight = true;
                        std::string s = coordinates_to_square(king_col, king_col);
                        std::cout << "Check by rook or queen " << s << "\n";
                        break;
                    }
                    if (board[rook_row][rook_col] != EMPTY){
                        break;
                    }
                }
            }
        }
}