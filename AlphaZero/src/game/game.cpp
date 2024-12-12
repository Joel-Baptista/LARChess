#include "../headers/game.h"

const char *square_to_coordinates[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};

int knight_jumps[5][5] =
{
    {-1,  0, -1,  1, -1},
    { 7, -1, -1, -1,  2},
    {-1, -1, -1, -1, -1},
    { 6, -1, -1, -1,  3},
    {-1,  5, -1,  4, -1},
};

int kj_row[8] =
{
     2, // 0 -> [4][1]
     2, // 1 -> [4][3]
     1, // 2 -> [3][4]
    -1, // 3 -> [1][4]
    -2, // 4 -> [0][3]
    -2, // 5 -> [0][1]
    -1, // 6 -> [1][0]
     1  // 7 -> [3][0]
};

int kj_cols[8] =
{
    -1, // 0 -> [4][1]
     1, // 1 -> [4][3]
     2, // 2 -> [3][4]
     2, // 3 -> [1][4]
     1, // 4 -> [0][3]
    -1, // 5 -> [0][1]
    -2, // 6 -> [1][0]
    -2  // 7 -> [3][0]
};

Game::Game()
{
    m_Board = std::make_unique<BitBoard>();
}

Game::~Game()
{
}

void Game::set_state(state current_state)
{
    m_Board->set_bitboards(current_state.bitboards);
    m_Board->set_occupancies(current_state.occupancies);
    m_Board->set_side(current_state.side);
    m_Board->set_en_passant_square(current_state.en_passant_square);
    m_Board->set_castle_rights(current_state.castle_rights);
    m_Board->set_halfmove(current_state.halfmove);
    m_Board->set_fullmove(current_state.fullmove);
}

state Game::get_state()
{
    state current_state;

    copy_state_from_board(current_state, m_Board);

    return current_state;
}

state Game::get_next_state(state current_state, std::string action)
{
    state new_state;

    set_state(current_state);

    copy_alpha_board(m_Board);

    int move = m_Board->parse_move(action.c_str());
    
    if (m_Board->make_move(move))
    {
        copy_state_from_board(new_state, m_Board);
    }

    restore_alpha_board(m_Board);

    return new_state;
}

void Game::get_encoded_state(torch::Tensor& encoded_state, state& current_state)
{
    if (current_state.side == 0) // White to move
        encoded_state[0][0].fill_(1);

    if (current_state.castle_rights & 1) // White can castle kingside
        encoded_state[0][1].fill_(1);

    if (current_state.castle_rights & 2) // White can castle queenside
        encoded_state[0][2].fill_(1);

    if (current_state.castle_rights & 4) // Black can castle kingside
        encoded_state[0][3].fill_(1);

    if (current_state.castle_rights & 8) // Black can castle queenside
        encoded_state[0][4].fill_(1);

    if (current_state.halfmove > 100)
        encoded_state[0][5].fill_(1);
    
    int idxsP[12];

    if (current_state.side == 0)
    {
        idxsP[0] = 0; idxsP[1] = 1; idxsP[2] = 2; idxsP[3] = 3; idxsP[4] = 4; idxsP[5] = 5;
        idxsP[6] = 6; idxsP[7] = 7; idxsP[8] = 8; idxsP[9] = 9; idxsP[10] = 10; idxsP[11] = 11; 
    }
    else
    {
        idxsP[0] = 6; idxsP[1] = 7; idxsP[2] = 8; idxsP[3] = 9; idxsP[4] = 10; idxsP[5] = 11;
        idxsP[6] = 0; idxsP[7] = 1; idxsP[8] = 2; idxsP[9] = 3; idxsP[10] = 4; idxsP[11] = 5; 
    }


    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;
           
            int piece = -1;
            for (int i = 0; i <= 11; i++)
            {
                if (get_bit(current_state.bitboards[i], square))
                {
                    piece = i;
                    break;
                }
            }

            if (piece != -1)
            {
                if (current_state.side == 0)
                    encoded_state[0][idxsP[piece] + 6][rank][file] = 1;
                else
                    encoded_state[0][idxsP[piece] + 6][7 - rank][file] = 1;
            }
        }
    }

    if (current_state.en_passant_square != 64)
    {
        int file = current_state.en_passant_square % 8;
        int rank = current_state.en_passant_square / 8;

        if (current_state.side == 0)
            encoded_state[0][18][rank][file] = 1;
        else
            encoded_state[0][18][7 - rank][file] = 1;
    }

    // for (int i = 0; i < 19; i++)
    // {
    //     std::cout << "New Plane " << i  << std::endl;
    //     for (int j = 0; j < 8; j++)
    //     {
    //         for (int k = 0; k < 8; k++)
    //         {
    //             std::cout << encoded_state[0][i][j][k].item() << " ";
    //         }
    //         std::cout << std::endl;
    //     }
    //     getchar();
    // }

}

void Game::get_valid_moves_encoded(torch::Tensor& encoded_valid_moves, state& current_state)
{
    set_state(current_state);

    moves move_list;
    m_Board->get_alpha_moves(&move_list);
    
    // std::cout << "Number of moves: " << move_list.count << std::endl;

    for (int i = 0; i < move_list.count; i++)
    {
        int source_square = get_alpha_move_source(move_list.moves[i]);
        int direction = get_alpha_move_direction(move_list.moves[i]);
        int length = get_alpha_move_length(move_list.moves[i]);
        int knight_move = get_alpha_move_knight(move_list.moves[i]);
        int underpromote = get_alpha_move_undepromotion(move_list.moves[i]);
        int is_knight = get_alpha_move_is_knight(move_list.moves[i]);
        int side = get_alpha_move_side(move_list.moves[i]);

        int index_plane = direction * 7 + length - 1;

        int row = (side == 0) ? source_square / 8 : 7 - source_square / 8;
        int col = (side == 0) ? source_square % 8 :  source_square % 8;

        if (is_knight)
            encoded_valid_moves[row][col][56 + knight_move] = 1.0f;
        else if (underpromote)
            encoded_valid_moves[row][col][64 + (underpromote - 1)] = 1.0f;
        else
            encoded_valid_moves[row][col][index_plane] = 1.0f;


        // std::cout << "Source Square: " << square_to_coordinates[source_square] << std::endl;
        // std::cout << "Direction: " << direction << std::endl;
        // std::cout << "Length: " << length << std::endl;
        // std::cout << "Knight Move: " << knight_move << std::endl;
        // std::cout << "Underpromote: " << underpromote << std::endl;
        // std::cout << "Is Knight: " << is_knight << std::endl;
        // std::cout << "Side: " << side << std::endl;
        // std::cout << "Row: " << row << std::endl;
        // std::cout << "Col: " << col << std::endl;
        // std::cout << "Index Plane: " << index_plane << std::endl;
        // std::cout << "Knight Plane: " << 56 + knight_move << std::endl;
        // std::cout << "Knight Plane: " << 64 + (underpromote - 1) << std::endl;

        // getchar();

    }
    // m_Board->print_board();
    // int count = 0;
    // for (int i = 0; i < 73; i++)
    // {
    //     // std::cout << "\nNew Plane " << i  << std::endl;
    //     for (int j = 0; j < 8; j++)
    //     {
    //         for (int k = 0; k < 8; k++)
    //         {
    //             // std::cout << encoded_valid_moves[j][k][i].item() << " ";
    //             if (encoded_valid_moves[j][k][i].item<float>() == 1.0f)
    //             {
    //                 count++;
    //             }
    //         }
    //         // std::cout << std::endl;
    //     }
    //     // std::cout << std::endl;
    //     // getchar();
    // }

    // std::cout << "Number of encoded valid moves: " << count << std::endl;
}

torch::Tensor Game::get_encoded_action(std::string move, int side)
{

    std::vector<int64_t> shape = {8, 8, 73};
    torch::Tensor encoded_move = torch::zeros(shape, torch::kFloat32); // Initialize the tensor with zeros

    // Make move player agnostic

    int source_row = (side == 0) ? 7 - (move[1] - '0' - 1) : (move[1] - '0' - 1); 
    int dest_row = (side == 0) ? 7 - (move[3] - '0' - 1) : (move[3] - '0' - 1);
    int source_col =  (int)(move[0] - 'a');
    int dest_col =  (int)(move[2] - 'a');

    char promotion = ' ';
    if (move.size() == 5)
    {
        promotion = move[4];
    }

    int delta_row = dest_row - source_row;
    int delta_col = dest_col - source_col;

    if (promotion == 'r' || promotion == 'n' || promotion == 'b') // just underpromotions
    {
        if (delta_col == 1)
        {   
            if (promotion == 'n')
                encoded_move[source_row][source_col][64] =1.0f;
            else if (promotion == 'b')
                encoded_move[source_row][source_col][67] =1.0f;
            else if (promotion == 'r')
                encoded_move[source_row][source_col][70] =1.0f;

            return encoded_move;
        }
        if (delta_col == 0)
        {   
            if (promotion == 'n')
                encoded_move[source_row][source_col][65] =1.0f;
            else if (promotion == 'b')
                encoded_move[source_row][source_col][68] =1.0f;
            else if (promotion == 'r')
                encoded_move[source_row][source_col][71] =1.0f;

            return encoded_move;
        }
        if (delta_col == -1)
        {   
            if (promotion == 'n')
                encoded_move[source_row][source_col][66] =1.0f;
            else if (promotion == 'b')
                encoded_move[source_row][source_col][69] =1.0f;
            else if (promotion == 'r')
                encoded_move[source_row][source_col][72] =1.0f;

            return encoded_move;
        }
    }

    if (delta_row == 0) // Horizontal moves
    {
        if (delta_col > 0)
        {
            encoded_move[source_row][source_col][2 * 7 + (abs(delta_col) - 1)] =1.0f;
        }
        else
        {
            encoded_move[source_row][source_col][6 * 7 + (abs(delta_col) - 1)] =1.0f;
        }

        return encoded_move;
    }

    if (delta_col == 0) // Vertical Moves
    {
        if (delta_row > 0)
        {
            encoded_move[source_row][source_col][4 * 7 + (abs(delta_row) - 1)] = 1.0f;
        }
        else
        {
            encoded_move[source_row][source_col][0 + (abs(delta_row) - 1)] = 1.0f;
        }

        return encoded_move;
    }

    if (abs(delta_col) != abs(delta_row)) // Knight jumps
    {
        int knight_plane = knight_jumps[delta_row+2][delta_col+2];

        encoded_move[source_row][source_col][56 + knight_plane] = 1;
        return encoded_move;
    }

    if (delta_row == delta_col) // Diagonal Moves
    {
        if (delta_row > 0)
        {
            encoded_move[source_row][source_col][3 * 7 + (abs(delta_row) - 1)] = 1.0f;
        }
        else
        {
            encoded_move[source_row][source_col][7 * 7 + (abs(delta_row) - 1)] = 1.0f;
        }

        return encoded_move;
    }
    else
    {
        if (delta_row > 0)
        {
            encoded_move[source_row][source_col][5 * 7 + (abs(delta_row) - 1)] = 1.0f;
        }
        else
        {
            encoded_move[source_row][source_col][1 * 7 + (abs(delta_row) - 1)] = 1.0f;
        }

        return encoded_move;
    }

    std::cout << "Error in encoding move" << std::endl;
    return encoded_move;

}

final_state Game::get_next_state_and_value(state current_state, std::string action, std::unordered_map<BitboardKey, int, BitboardHash>& state_counter)
{
    state new_state;
    final_state sFinal;
    sFinal.terminated = false;

    set_state(current_state);

    copy_alpha_board(m_Board);

    int move = m_Board->parse_move(action.c_str());
    
    if (m_Board->make_move(move))
    {
        copy_state_from_board(new_state, m_Board);
    }

    sFinal.board_state = new_state;

    if (m_Board->get_halfmove() >= 100 || state_counter[new_state.bitboards] >= 2 || insufficient_material(new_state) || m_Board->get_fullmove() > 256)
    {
        sFinal.value = 0.0f;
        sFinal.terminated = true;
        restore_alpha_board(m_Board);
        return sFinal;
    }

    int valid_state_count = 0;

    moves move_list;
    m_Board->get_generate_moves(&move_list);

    for (int i = 0; i < move_list.count; i++)
    {
        copy_alpha_board(m_Board);

        if (m_Board->make_move(move_list.moves[i]))
        {
            valid_state_count++;
            restore_alpha_board(m_Board);
        }

        restore_alpha_board(m_Board);
    }
    if (valid_state_count == 0)
    {
        int side = m_Board->get_side();
        int is_check = m_Board->get_is_square_attacked(get_least_significant_bit((side == 0) ? m_Board->get_bitboard(5) : m_Board->get_bitboard(11)) , side ^ 1);

        if (is_check)
        {
            sFinal.value = 1.0f;
            sFinal.terminated = true;
            restore_alpha_board(m_Board);
            return sFinal;
        }

        sFinal.value = 0.0f;
        sFinal.terminated = true;
        
        restore_alpha_board(m_Board);

        return sFinal;

    }

    restore_alpha_board(m_Board);

    return sFinal;
}

final_state Game::get_value_and_terminated(state current_state, std::unordered_map<BitboardKey, int, BitboardHash>& state_counter)
{
    set_state(current_state);
    final_state sFinal;
    sFinal.terminated = false;

    if (m_Board->get_halfmove() >= 100 || state_counter[current_state.bitboards] >= 2 || insufficient_material(current_state))
    {
        sFinal.value = 0.0f;
        sFinal.terminated = true;
        return sFinal;
    }

    int valid_state_count = 0;

    moves move_list;
    m_Board->get_generate_moves(&move_list);

    for (int i = 0; i < move_list.count; i++) // Check if there are valid moves
    {
        copy_alpha_board(m_Board);

        if (m_Board->make_move(move_list.moves[i]))
        {
            valid_state_count++;
            restore_alpha_board(m_Board);
        }

        restore_alpha_board(m_Board);
    }
    if (valid_state_count == 0)
    {
        int side = m_Board->get_side();
        int is_check = m_Board->get_is_square_attacked(get_least_significant_bit((side == 0) ? m_Board->get_bitboard(5) : m_Board->get_bitboard(11)) , side ^ 1);

        if (is_check)
        {
            sFinal.value = -1.0f; // If you are in this state then you are mated
            sFinal.terminated = true;
            return sFinal;
        }

        sFinal.value = 0.0f;
        sFinal.terminated = true;
        
        return sFinal;

    }

    return sFinal;
}

std::string Game::decode_action(state current_state, torch::Tensor action)
{   
    float maximum = -1;
    int max_row = 0;
    int max_col = 0;
    int max_plane = 0;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            for (int k = 0; k < 73; k++)
            {
                if (action[i][j][k].item<float>() > maximum)
                {
                    maximum = action[i][j][k].item<float>();
                    max_row = i;
                    max_col = j;
                    max_plane = k;
                }
            }
        }
    }

    // std::cout << "Maximum: " << maximum << " in position (" << max_row << ", " << max_col << ", " << max_plane << ") for player " << current_state.side << std::endl;

    int index_dir = max_plane / 7;
    int index_knight = (max_plane - 56) % 8;
    int index_length = (max_plane % 7) + 1;

    int source_square = (current_state.side == 0) ? max_row * 8 + max_col : (7 - max_row) * 8 + max_col;

    std::string move = square_to_coordinates[source_square];
    std::string dest_square;

    if (index_dir == 0)
    {  
        dest_square = square_to_coordinates[(current_state.side == 0) ? 
                    (max_row - index_length) * 8 + max_col : 
                    (7 - max_row + index_length) * 8 + max_col];
        
        if ((dest_square[1] == '1' && current_state.side == 1) || (dest_square[1] == '8' && current_state.side == 0))
            dest_square += "q"; 
    }
    else if (index_dir == 1)
    {
        dest_square = square_to_coordinates[(current_state.side == 0) ? 
                        (max_row - index_length) * 8 + (max_col + index_length) : 
                        (7 - max_row + index_length) * 8 + (max_col + index_length)];

        if ((dest_square[1] == '1' && current_state.side == 1) || (dest_square[1] == '8' && current_state.side == 0))
            dest_square += "q";
    }
    else if (index_dir == 2)
    {
        dest_square = square_to_coordinates[(current_state.side == 0) ? 
            (max_row) * 8 + (max_col + index_length) : 
            (7 - max_row) * 8 + (max_col + index_length)];
    }
    else if (index_dir == 3)
    {
        dest_square = square_to_coordinates[(current_state.side == 0) ? 
            (max_row + index_length) * 8 + (max_col + index_length) : 
            (7 - max_row - index_length) * 8 + (max_col + index_length)];
    }
    else if (index_dir == 4)
    {
        
        dest_square = square_to_coordinates[(current_state.side == 0) ? 
            (max_row + index_length) * 8 + (max_col) : 
            (7 - max_row - index_length) * 8 + (max_col)];
    }
    else if (index_dir == 5)
    {
        dest_square = square_to_coordinates[(current_state.side == 0) ? 
            (max_row + index_length) * 8 + (max_col - index_length) : 
            (7 - max_row - index_length) * 8 + (max_col - index_length)];
    }
    else if (index_dir == 6)
    {
        dest_square = square_to_coordinates[(current_state.side == 0) ? 
            (max_row) * 8 + (max_col - index_length) : 
            (7 - max_row) * 8 + (max_col - index_length)];
    }
    else if (index_dir == 7)
    {
        dest_square = square_to_coordinates[(current_state.side == 0) ? 
            (max_row - index_length) * 8 + (max_col - index_length) : 
            (7 - max_row + index_length) * 8 + (max_col - index_length)];

        if ((dest_square[1] == '1' && current_state.side == 1) || (dest_square[1] == '8' && current_state.side == 0))
            dest_square += "q";
    }
    else if ( max_plane >= 56 && max_plane < 64) // Knights Jumps
    {
        dest_square = square_to_coordinates[(current_state.side == 0) ? 
            (max_row - kj_row[index_knight]) * 8 + (max_col + kj_cols[index_knight]) : 
            (7 - max_row + kj_row[index_knight]) * 8 + (max_col + kj_cols[index_knight])];
    }
    else // Underpromotion
    {
        int underpromote = max_plane - 64;

        if (underpromote % 3 == 0)
            dest_square = square_to_coordinates[(current_state.side == 0) ? 
                    (max_row - 1) * 8 + (max_col + 1) : 
                    (7 - max_row + 1) * 8 + (max_col + 1)];

        if (underpromote % 3 == 1)
            dest_square = square_to_coordinates[(current_state.side == 0) ? 
                    (max_row - 1) * 8 + max_col : 
                    (7 - max_row + 1) * 8 + max_col];
        
        if (underpromote % 3 == 2)
            dest_square = square_to_coordinates[(current_state.side == 0) ? 
                (max_row - 1) * 8 + (max_col - 1) : 
                (7 - max_row + 1) * 8 + (max_col - 1)];

        if (underpromote >=0 && underpromote < 3) dest_square += "n";
        if (underpromote >=3 && underpromote < 6) dest_square += "b";
        if (underpromote >=6 && underpromote < 9) dest_square += "r";
    }

    move += dest_square;

    return move;

}

std::vector<decoded_action> Game::decode_actions(state current_state, torch::Tensor action, torch::Tensor valid_moves)
{
    std::vector<decoded_action> actions;
    int count = 0;

    auto action_indexs = torch::nonzero(action);

    for (int i = 0; i < action_indexs.sizes()[0]; i++)
    {
        count++;
        decoded_action dAction;
        dAction.action = "";
        dAction.probability = 0.0f;

        if (action_indexs.sizes()[1] < 3) {
            std::cerr << "Invalid action tensor indices with size: " << action_indexs.sizes() << std::endl;
            continue;
        }

        int plane = action_indexs[i][2].item<int>();
        int col = action_indexs[i][1].item<int>();
        int row = action_indexs[i][0].item<int>();

        int index_dir = plane / 7;
        int index_knight = (plane - 56) % 8;
        int index_length = (plane % 7) + 1;

        int source_square = (current_state.side == 0) ? row * 8 + col : (7 - row) * 8 + col;

        std::string move = square_to_coordinates[source_square];
        std::string dest_square;

        if (!(source_square >= 0 && source_square < 64)) {
            std::cerr << "Invalid source square" << std::endl;
            continue;
        }

        switch (index_dir)
        {
            case 0:
            {
                dest_square = square_to_coordinates[(current_state.side == 0) ? 
                            (row - index_length) * 8 + col : 
                            (7 - row + index_length) * 8 + col];
                
                if ((dest_square[1] == '1' && current_state.side == 1) || (dest_square[1] == '8' && current_state.side == 0))
                    dest_square += "q"; 
                break;
            }   
            case 1:
            {
                dest_square = square_to_coordinates[(current_state.side == 0) ? 
                                (row - index_length) * 8 + (col + index_length) : 
                                (7 - row + index_length) * 8 + (col + index_length)];

                if ((dest_square[1] == '1' && current_state.side == 1) || (dest_square[1] == '8' && current_state.side == 0))
                    dest_square += "q";
                break;
            }
            case 2:
            {
                dest_square = square_to_coordinates[(current_state.side == 0) ? 
                    (row) * 8 + (col + index_length) : 
                    (7 - row) * 8 + (col + index_length)];
                break;
            }
            case 3:
            {
                dest_square = square_to_coordinates[(current_state.side == 0) ? 
                    (row + index_length) * 8 + (col + index_length) : 
                    (7 - row - index_length) * 8 + (col + index_length)];
                break;
            }
            case 4:
            {
                dest_square = square_to_coordinates[(current_state.side == 0) ? 
                    (row + index_length) * 8 + (col) : 
                    (7 - row - index_length) * 8 + (col)];
                break;
            }
            case 5:
            {
                dest_square = square_to_coordinates[(current_state.side == 0) ? 
                    (row + index_length) * 8 + (col - index_length) : 
                    (7 - row - index_length) * 8 + (col - index_length)];
                break;
            }
                
            case 6:
            {
                dest_square = square_to_coordinates[(current_state.side == 0) ? 
                    (row) * 8 + (col - index_length) : 
                    (7 - row) * 8 + (col - index_length)];
                break;
            }
            case 7:
            {
                dest_square = square_to_coordinates[(current_state.side == 0) ? 
                    (row - index_length) * 8 + (col - index_length) : 
                    (7 - row + index_length) * 8 + (col - index_length)];

                if ((dest_square[1] == '1' && current_state.side == 1) || (dest_square[1] == '8' && current_state.side == 0))
                    dest_square += "q";
                break;
            }
                
            default:
            {
                if ( plane >= 56 && plane < 64) // Knights Jumps
                {
                    dest_square = square_to_coordinates[(current_state.side == 0) ? 
                        (row - kj_row[index_knight]) * 8 + (col + kj_cols[index_knight]) : 
                        (7 - row + kj_row[index_knight]) * 8 + (col + kj_cols[index_knight])];
                }
                else // Underpromotion
                {
                    int underpromote = plane - 64;

                    if (underpromote % 3 == 0)
                        dest_square = square_to_coordinates[(current_state.side == 0) ? 
                                (row - 1) * 8 + (col + 1) : 
                                (7 - row + 1) * 8 + (col + 1)];

                    if (underpromote % 3 == 1)
                        dest_square = square_to_coordinates[(current_state.side == 0) ? 
                                (row - 1) * 8 + col : 
                                (7 - row + 1) * 8 + col];
                    
                    if (underpromote % 3 == 2)
                        dest_square = square_to_coordinates[(current_state.side == 0) ? 
                            (row - 1) * 8 + (col - 1) : 
                            (7 - row + 1) * 8 + (col - 1)];

                    if (underpromote >=0 && underpromote < 3) dest_square += "n";
                    if (underpromote >=3 && underpromote < 6) dest_square += "b";
                    if (underpromote >=6 && underpromote < 9) dest_square += "r";
                }
                break;
            }
        }

        move += dest_square;

        dAction.probability = action[row][col][plane].item<float>();
        dAction.action = move;

        actions.push_back(dAction);
    }

    return actions;
}

void Game::reset_board()
{
    m_Board->parse_fen(start_position);
}

bool Game::insufficient_material(state current_state)
{

    int white_knight_count = 0;
    int black_knight_count = 0;
    int white_bishop_count = 0;
    int black_bishop_count = 0;

    for (int i = 0; i < 12; i++)
    {
        for (int j = 0; j < 64 ; j++)
        {
            if (get_bit(current_state.bitboards[i], j))
            {
                if (i == 1)
                    white_knight_count++;
                if (i == 2)
                    white_bishop_count++;
                if (i == 7)
                    black_knight_count++;
                if (i == 8)
                    black_bishop_count++;
                if (i == 3 || i == 9) return false;; // There is a rook
                if (i == 0 || i == 6) return false; // Threre is a pawn
                if (i == 4 || i == 10) return false; // There is a queen
            }
        }
    }

    if (white_bishop_count > 1 || black_bishop_count > 1) return false; // 2 Bishops vs King
    if ((white_bishop_count >= 1 && white_knight_count >= 1) || (black_bishop_count >= 1 && black_knight_count >= 1)) return false; // Bishop and Knight vs King
    if ((white_knight_count == 2 && (black_bishop_count >= 1 || black_knight_count >= 1)) || 
        (black_knight_count == 2 && (white_bishop_count >= 1 || white_knight_count >= 1))) return false; // 2 Knights vs King and minor piece

    return true;

}
