#pragma once

#include "../../BBChessEngine/src/bit_board.h"
#include <memory>
#include "include/xtensor/xarray.hpp"
#include "include/xtensor/xio.hpp"
#include "include/xtensor/xview.hpp"
#include "include/xtensor/xadapt.hpp"

#include "include/xtensor/xtensor.hpp"

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

        xt::xtensor<float, 3> get_encoded_state(state current_state);
        xt::xtensor<float, 3> get_valid_moves_encoded(state current_state);
        xt::xtensor<float, 3> get_encoded_action(std::string move, int side);
        std::vector<decoded_action> decode_actions(state current_state, xt::xtensor<float, 3> action);
        state get_next_state(state current_state, std::string action);
        void set_state(state current_state);
        final_state get_value_and_terminated(state current_state);
        std::string decode_action(state current_state, xt::xtensor<float, 3> action);
        final_state get_next_state_and_value(state current_state, std::string action);
    
        void get_opponent_value();
        void get_board_state();
        std::unique_ptr<BitBoard> m_Board;

    private:


};
