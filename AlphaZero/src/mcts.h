#pragma once

#include "../../BBChessEngine/src/bit_board.h"
#include <vector>
#include "game.h"
#include <array>
#include <memory>

#include "include/xtensor/xarray.hpp"
#include "include/xtensor/xio.hpp"
#include "include/xtensor/xview.hpp"
#include "include/xtensor/xadapt.hpp"

#include "include/xtensor/xtensor.hpp"

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


#define  matrix4D(b, c, h, w, type) std::array<std::array<std::array<std::array<type, w>, h>, c>, b>


class Node
{
    public:
        Node(BitBoard* board, Node* parent, std::string action, float C, float prior, int visit_count);
        ~Node();

        bool is_fully_expanded();
        Node* select();
        float get_ubc(Node* child);
        void expand(xt::xtensor<float, 3> action_probs);
        void backpropagate(float value);

        // Pass this variables to private once finished debugging
        std::vector<Node*> pChildren; 
        std::string action;
        int visit_count; 

    private:

        BitBoard* game;
        state node_state;
        float value_sum;
        Node* parent;
        float C;
        float prior;

};

struct memory_item
{
    state board_state;
    xt::xtensor<float, 3> action_probs;
    int player;
};

class SPG
{
    public:
        SPG(BitBoard* board);
        ~SPG();

        state initial_state;
        state current_state;

        std::vector<memory_item> memory;

        Node* pRoot;
        Node* pCurrentNode;
        BitBoard* game;

    private:


};

class MCTS
{
    public:
        MCTS(int num_searches, float dichirlet_alpha, float dichirlet_epsilon, float C);
        ~MCTS();
    
    void search(std::vector<SPG*>* spGames);

    private:
        int num_searches;
        float dichirlet_alpha;
        float dichirlet_epsilon;

        float C;

};


