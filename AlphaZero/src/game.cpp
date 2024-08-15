#include "game.h"

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

state Game::get_next_state(state current_state, std::string action)
{
    state new_state;

    set_state(current_state);

    copy_alpha_board(m_Board);

    int move = m_Board->parse_move(action.c_str());
    
    
    if (m_Board->make_move(move, 0))
    {
        copy_state_from_board(new_state, m_Board);
    }

    restore_alpha_board(m_Board);

    return new_state;
}



xt::xtensor<float, 4> Game::get_encoded_state(state current_state)
{
    std::array<std::size_t, 4> shape = {1, 19, 8, 8}; 
    xt::xtensor<float, 4> encoded_state(shape);
    encoded_state.fill(0.0f);

    if (current_state.side == 0) // White to move
        xt::view(encoded_state, 0, 0, xt::range(0, 8)) = 1;

    if (current_state.castle_rights & 1) // White can castle kingside
        xt::view(encoded_state, 0, 1, xt::range(0, 8)) = 1;
    
    if (current_state.castle_rights & 2) // White can castle queenside
        xt::view(encoded_state, 0, 2, xt::range(0, 8)) = 1;
    
    if (current_state.castle_rights & 4) // Black can castle kingside
        xt::view(encoded_state, 0, 3, xt::range(0, 8)) = 1;

    if (current_state.castle_rights & 8) // Black can castle queenside
        xt::view(encoded_state, 0, 4, xt::range(0, 8)) = 1;

    if (current_state.halfmove > 100)
        xt::view(encoded_state, 0, 5, xt::range(0, 8)) = 1;

    
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
                    encoded_state(0, idxsP[piece] + 6, rank, file) = 1;
                else
                    encoded_state(0, idxsP[piece] + 6, 7 - rank, file) = 1;
            }
        }
    }

    if (current_state.en_passant_square != 64)
    {
        int file = current_state.en_passant_square % 8;
        int rank = current_state.en_passant_square / 8;

        if (current_state.side == 0)
            encoded_state(0, 18, rank, file) = 1;
        else
            encoded_state(0, 18, 7 - rank, file) = 1;
    }

    // for (int i = 0; i < 19; i++)
    // {
    //     std::cout << "New Plane " << i  << std::endl;
    //     for (int j = 0; j < 8; j++)
    //     {
    //         for (int k = 0; k < 8; k++)
    //         {
    //             std::cout << encoded_state(0, i, j, k) << " ";
    //         }
    //         std::cout << std::endl;
    //     }
    //     getchar();
    // }

    return encoded_state;
}

xt::xtensor<float, 4> Game::get_valid_moves_encoded(state current_state)
{
    set_state(current_state);

    moves move_list;
    m_Board->get_alpha_moves(&move_list);

    std::array<std::size_t, 4> shape = {1, 8, 8, 73};
    xt::xtensor<float, 4> encoded_valid_moves(shape);

    std::cout << "Number of moves: " << move_list.count << std::endl;

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
        int col = (side == 0) ? source_square % 8 :  7 - source_square % 8;

        if (is_knight)
            encoded_valid_moves(0, row, col, 56 + knight_move) = 1;
        else if (underpromote)
            encoded_valid_moves(0, row, col, 64 + (underpromote - 1)) = 1;
        else
            encoded_valid_moves(0, row, col, index_plane) = 1;


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

    // int count = 0;
    // for (int i = 0; i < 73; i++)
    // {
    //     std::cout << "\nNew Plane " << i  << std::endl;
    //     for (int j = 0; j < 8; j++)
    //     {
    //         for (int k = 0; k < 8; k++)
    //         {
    //             std::cout << encoded_valid_moves(0, j, k, i) << " ";
    //             if (encoded_valid_moves(0, j, k, i) == 1)
    //             {
    //                 count++;
    //             }
    //         }
    //         std::cout << std::endl;
    //     }
    //     std::cout << std::endl;
    //     getchar();
    // }

    // std::cout << "Total moves: " << count << std::endl;

    return encoded_valid_moves;
}

xt::xtensor<float, 4> Game::get_encoded_action(std::string move, int side)
{

    std::array<std::size_t, 4> shape = {1, 8, 8, 73};
    xt::xtensor<float, 4> encoded_move(shape);
    encoded_move.fill(0.0f);

    // Make move player agnostic
    int source_col = (side == 0) ? move[0] - 'a' : 7 - (move[0] - 'a') ;
    int source_row = (side == 0) ? 8 - (move[1] - '0') : (move[1] - '0' - 1);
    int dest_col = (side == 0) ? (move[2] - 'a') : 7 - (move[2] - 'a');
    int dest_row = (side == 0) ? 8 -  (move[3] - '0') : (move[3] - '0' - 1);
    char promotion = ' ';
    if (move.size() == 5)
    {
        promotion = move[4];
    }

    std::cout << "Source Row: " << source_row << std::endl;
    std::cout << "Source Col: " << source_col << std::endl;
    std::cout << "Dest Row: " << dest_row << std::endl;
    std::cout << "Dest Col: " << dest_col << std::endl;

    int delta_row = dest_row - source_row;
    int delta_col = dest_col - source_col;

    if (promotion == 'r' || promotion == 'n' || promotion == 'b') // just underpromotions
    {
        if (delta_col == 1)
        {   
            if (promotion == 'n')
                encoded_move(0, source_row, source_col, 64) = 1;
            else if (promotion == 'b')
                encoded_move(0, source_row, source_col, 67) = 1;
            else if (promotion == 'r')
                encoded_move(0, source_row, source_col, 70) = 1;

            return encoded_move;
        }
        if (delta_col == 0)
        {   
            if (promotion == 'n')
                encoded_move(0, source_row, source_col, 65) = 1;
            else if (promotion == 'b')
                encoded_move(0, source_row, source_col, 68) = 1;
            else if (promotion == 'r')
                encoded_move(0, source_row, source_col, 71) = 1;

            return encoded_move;
        }
        if (delta_col == -1)
        {   
            if (promotion == 'n')
                encoded_move(0, source_row, source_col, 66) = 1;
            else if (promotion == 'b')
                encoded_move(0, source_row, source_col, 69) = 1;
            else if (promotion == 'r')
                encoded_move(0, source_row, source_col, 72) = 1;

            return encoded_move;
        }
    }

    if (delta_row == 0) // Horizontal moves
    {
        if (delta_col > 0)
        {
            encoded_move(0, source_row, source_col, 14 + (abs(delta_col) - 1)) = 1;
        }
        else
        {
            encoded_move(0, source_row, source_col, 6 * 7 + (abs(delta_col) - 1)) = 1;
        }

        return encoded_move;
    }

    if (delta_col == 0) // Vertical Moves
    {
        if (delta_row > 0)
        {
            encoded_move(0, source_row, source_col, 0 + (abs(delta_row) - 1)) = 1;
        }
        else
        {
            encoded_move(0, source_row, source_col, 4 * 7 + (abs(delta_row) - 1)) = 1;
        }

        return encoded_move;
    }

    if (abs(delta_col) != abs(delta_row)) // Knight jumps
    {
        int knight_plane = knight_jumps[delta_row+2][delta_col+2];
        std::cout << "Knight Plane: " << knight_plane << std::endl;

        encoded_move(0, source_row, source_col, 56 + knight_plane) = 1;
        return encoded_move;
    }

    if (delta_row == delta_col) // Diagonal Moves
    {
        if (delta_row > 0)
        {
            encoded_move(0, source_row, source_col, 7 + (abs(delta_row) - 1)) = 1;
        }
        else
        {
            encoded_move(0, source_row, source_col, 5 * 7 + (abs(delta_row) - 1)) = 1;
        }

        return encoded_move;
    }
    else
    {
        if (delta_row > 0)
        {
            encoded_move(0, source_row, source_col, 7 * 7 + (abs(delta_row) - 1)) = 1;
        }
        else
        {
            encoded_move(0, source_row, source_col, 3 * 7 + (abs(delta_row) - 1)) = 1;
        }

        return encoded_move;
    }


    return encoded_move;

}

final_state Game::get_value_and_terminated(state current_state, std::string action)
{
    set_state(current_state);
    final_state final;
    final.terminated = false;

    m_Board->make_move(m_Board->parse_move(action.c_str()), 0);

    if (m_Board->get_halfmove() >= 100)
    {
        final.value = 0.0f;
        final.terminated = true;
        return final;
    }

    int valid_state_count = 0;

    moves move_list;
    m_Board->generate_moves(&move_list);

    for (int i = 0; i < move_list.count; i++)
    {
        copy_alpha_board(m_Board);

        if (m_Board->make_move(move_list.moves[i], 0))
        {
            valid_state_count++;
            restore_alpha_board(m_Board);
        }

        restore_alpha_board(m_Board);
    }
    if (valid_state_count == 0)
    {
        int side = m_Board->get_side();
        int is_check = m_Board->is_square_attacked(get_least_significant_bit((side == 0) ? m_Board->get_bitboard(5) : m_Board->get_bitboard(11)) , side ^ 1);

        if (is_check)
        {
            final.value = 1.0f;
            final.terminated = true;
            return final;
        }

        final.value = 0.0f;
        final.terminated = true;
        
        return final;

    }

    return final;
}