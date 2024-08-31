#pragma once

#include "../../BBChessEngine/src/bit_board.h"
#include <memory>
// #include "include/xtensor/xarray.hpp"
// #include "include/xtensor/xio.hpp"
// #include "include/xtensor/xview.hpp"
// #include "include/xtensor/xadapt.hpp"

// #include "include/xtensor/xtensor.hpp"

#include <torch/torch.h>
#include "include/ResNet.h"
#include "utils.h"


#define copy_state_from_board(dest, src)    memcpy(dest.bitboards, src->get_bitboards(), sizeof(dest.bitboards)); \
                                            memcpy(dest.occupancies, src->get_occupancies(), sizeof(dest.occupancies)); \
                                            dest.side = src->get_side(); \
                                            dest.en_passant_square = src->get_en_passant_square(); \
                                            dest.castle_rights = src->get_castle_rights(); \
                                            dest.halfmove = src->get_halfmove(); \
                                            dest.fullmove = src->get_fullmove();

#define copy_state(dest, src)    memcpy(dest.bitboards, src.bitboards, sizeof(src.bitboards)); \
                                            memcpy(dest.occupancies, src.occupancies, sizeof(src.occupancies)); \
                                            dest.side = src.side; \
                                            dest.en_passant_square = src.en_passant_square; \
                                            dest.castle_rights = src.castle_rights; \
                                            dest.halfmove = src.halfmove; \
                                            dest.fullmove = src.fullmove;

#define copy_alpha_board(src)                                                                   \
    U64 bitboards_copy[12], occupancies_copy[12];                                               \
    int side_copy, en_passant_square_copy, castle_rights_copy, halfmove_copy, fullmove_copy;    \
    memcpy(bitboards_copy, src->get_bitboards(), sizeof(bitboards_copy)); \
    memcpy(occupancies_copy, src->get_occupancies(), sizeof(occupancies_copy)); \
    side_copy = src->get_side(); \
    en_passant_square_copy = src->get_en_passant_square(); \
    castle_rights_copy = src->get_castle_rights(); \
    halfmove_copy = src->get_halfmove(); \
    fullmove_copy = src->get_fullmove();



#define restore_alpha_board(dest)                                        \
    dest->set_bitboards(bitboards_copy); \
    dest->set_occupancies(occupancies_copy); \
    dest->set_side(side_copy); \
    dest->set_en_passant_square(en_passant_square_copy); \
    dest->set_castle_rights(castle_rights_copy); \
    dest->set_halfmove(halfmove_copy); \
    dest->set_fullmove(fullmove_copy);




struct state
{
    U64 bitboards[12]; // 12 bitboards for each piece type (6 for white and 6 for black)
    U64 occupancies[3]; // occupancy bitboards for each color and all pieces
    int side; // side to move
    int en_passant_square = 64;
    int castle_rights; 
    int halfmove;
    int fullmove;
};

struct final_state
{
    state board_state;
    float value = 0.0f;
    bool terminated = false;
};

struct decoded_action
{
    std::string action;
    float probability;
};

class Game
{
    public:
        Game();
        ~Game();

        void get_encoded_state(torch::Tensor& encoded_state, state& current_state);
        void get_valid_moves_encoded(torch::Tensor& valid_encoded_actions, state& current_state);
        torch::Tensor get_encoded_action(std::string move, int side);
        std::vector<decoded_action> decode_actions(state current_state, torch::Tensor action, torch::Tensor valid_moves);
        state get_next_state(state current_state, std::string action);
        void set_state(state current_state);
        std::string decode_action(state current_state, torch::Tensor action);
        final_state get_value_and_terminated(state current_state, 
        std::unordered_map<BitboardKey, int, BitboardHash>& state_counter);
        final_state get_next_state_and_value(state current_state, std::string action, 
        std::unordered_map<BitboardKey, int, BitboardHash>& state_counter);
    
        void get_opponent_value();
        state get_state();
        std::unique_ptr<BitBoard> m_Board;
        void reset_board();

    private:

        bool insufficient_material(state current_state);


};

inline void get_encoded_state(torch::Tensor& encoded_state, state& current_state)
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

inline void get_valid_moves_encoded(torch::Tensor& encoded_valid_moves, state& current_state, moves& move_list)
{
    // set_state(current_state);

    // moves move_list;
    // m_Board->get_alpha_moves(&move_list);
    
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
}