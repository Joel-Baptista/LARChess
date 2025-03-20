#pragma once
#include <unordered_map>
#include <iostream>
#include <string>
#include <string.h>
#include <stdlib.h> 
#include <sys/time.h>
#include <array>
#include <cmath>

// Macros
#define U64 unsigned long long
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define clear_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))
#define encode_move(source, target, piece, promoted, capture, double_push, en_passant, castling) \
    ((source) | ((target) << 6) | ((piece) << 12) | ((promoted) << 16) | ((capture) << 20) | ((double_push) << 21) | ((en_passant) << 22) | ((castling) << 23))

#define get_move_source(move) ((move) & 0x3f)
#define get_move_target(move) (((move) & 0xfc0) >> 6)
#define get_move_piece(move) (((move) & 0xf000) >> 12)
#define get_move_promoted(move) (((move) & 0xf0000) >> 16)
#define get_move_capture(move) (((move) & 0x100000))
#define get_move_double(move) (((move) & 0x200000))
#define get_move_enpassant(move) (((move) & 0x400000))
#define get_move_castling(move) (((move) & 0x800000))


#define encode_alpha_move(source, direction, length, knight, underpromotion, is_knight, side) \
    ((source) | ((direction) << 6) | ((length) << 9) | ((knight) << 12) | ((underpromotion) << 15) | ((is_knight) << 20) | ((side) << 21))

#define get_alpha_move_source(move) ((move) & 0x3f)
#define get_alpha_move_direction(move) (((move) & 0x1c0) >> 6)
#define get_alpha_move_length(move) (((move) & 0xE00) >> 9)
#define get_alpha_move_knight(move) (((move) & 0x7000) >> 12)
#define get_alpha_move_undepromotion(move) (((move) & 0xF8000) >> 15)
#define get_alpha_move_is_knight(move) (((move) & 0x100000) >> 20)
#define get_alpha_move_side(move) (((move) & 0x200000) >> 21)




#define copy_board()                                                                        \
    U64 bitboards_copy[12], occupancies_copy[12];                                           \
    int side_copy, en_passant_square_copy, castle_rights_copy, halfmove_copy, fullmove_copy;\
    memcpy(bitboards_copy, bitboards, sizeof(bitboards));                                   \
    memcpy(occupancies_copy, occupancies, sizeof(occupancies));                             \
    side_copy = side;                                                                       \
    en_passant_square_copy = en_passant_square;                                             \
    castle_rights_copy = castle_rights;                                                     \
    halfmove_copy = halfmove;                                                               \
    fullmove_copy = fullmove;                                                               \


#define restore_board()                                             \
    memcpy(bitboards, bitboards_copy, sizeof(bitboards));           \
    memcpy(occupancies, occupancies_copy, sizeof(occupancies));     \
    side = side_copy;                                               \
    en_passant_square = en_passant_square_copy;                     \
    castle_rights = castle_rights_copy;                             \
    halfmove = halfmove_copy;                                       \
    fullmove = fullmove_copy;                                       \


// Some test strings
#define empty_board "8/8/8/8/8/8/8/8 w - -"
#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
#define killer_position "rnbqkb1r/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 w KQkq e6 0 1"
#define cmk_position "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9"

typedef struct {
    int moves[256]; // The average number of legal moves is 40, so 256 is a good number
    int count = 0; // Number of moves
} moves;

const int bishop_relevant_bits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    6, 5, 5, 5, 5, 5, 5, 6, 
};

const int rook_relevant_bits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    12, 11, 11, 11, 11, 11, 11, 12,
};


int get_time_ms();

static inline void add_move(moves *move_list, int move) // Does not need to be a member function
{
    move_list->moves[move_list->count] = move;
    move_list->count++;
}
// Bit Manipulation Functions

static inline int count_bits(U64 bitboard)
{
    // int count = 0;
    // while (bitboard)
    // {
    //     count++;
    //     bitboard &= bitboard - 1; // Remove least significant bit set
    // }
    return __builtin_popcountll(bitboard); // Just use built-in function
}

static inline int get_least_significant_bit(U64 bitboard)
{
    if (bitboard)
    {
        // return count_bits((bitboard & -bitboard) - 1);
        return __builtin_ctzll(bitboard); // Just use built-in function
    }
    else
    {
        return -1;
    }

}


class BitBoard {
    public:
        BitBoard();
        BitBoard(const BitBoard& other)
        {

            memcpy(bitboards, other.bitboards, sizeof(bitboards));
            memcpy(occupancies, other.occupancies, sizeof(occupancies));
            memcpy(pawn_attacks, other.pawn_attacks, sizeof(pawn_attacks));
            memcpy(knight_attacks, other.knight_attacks, sizeof(knight_attacks));
            memcpy(king_attacks, other.king_attacks, sizeof(king_attacks));
            memcpy(bishop_attacks, other.bishop_attacks, sizeof(bishop_attacks));
            memcpy(bishop_masks, other.bishop_masks, sizeof(bishop_masks));
            memcpy(rook_attacks, other.rook_attacks, sizeof(rook_attacks));
            memcpy(rook_masks, other.rook_masks, sizeof(rook_masks));
            side = other.side;
            en_passant_square = other.en_passant_square;
            castle_rights = other.castle_rights;
            halfmove = other.halfmove;
            fullmove = other.fullmove;
        };
        
        ~BitBoard();
     
        BitBoard* clone() const {
            return new BitBoard(*this); // Uses the copy constructor
        }

        void print_bitboard(U64 bitboard);
        void print_board();
        void parse_fen(const char *fen);
        std::string get_fen();
        void print_move(int move);
        void print_move_list(moves* move_list);

        // Normal Getters and Setters

        // Testing
        void perft_driver(int depth);
        void perft_test(int depth);
        void reset_leaf_nodes() { leaf_nodes = 0; }
        long get_leaf_nodes() { return leaf_nodes; }

        // Bots

        std::string move_to_uci(int move);
        float alpha_beta(int depth, float alpha, float beta, bool quien = false);
        std::array<std::array<int, 8>, 8> bitboard_to_board();
        int make_player_move(const char *move);
        int make_bot_move(int move);
        int parse_move(const char* move_string);
        void get_alpha_moves(moves* move_list);
        void get_moves(moves* move_list);
        int make_move(int move) { return make_move(move, 0); }
        void get_generate_moves(moves* move_list) { generate_moves(move_list); }
        int get_is_square_attacked(int square, int side) { return is_square_attacked(square, side); }

        
        // Board Variables Interface
        U64 get_bitboard(int piece) { return bitboards[piece]; }
        // void get_bitboards(U64 new_bitboards[12]) { memcpy(new_bitboards, bitboards, sizeof(bitboards)); }
        // void get_occupancies(U64 new_occupancies[3]) { memcpy(new_occupancies, occupancies, sizeof(occupancies)); }
        
        const U64* get_bitboards() { return bitboards; }
        const U64* get_occupancies() { return occupancies; }
        
        int get_side() { return side; }
        int get_en_passant_square() { return en_passant_square; }
        int get_castle_rights() { return castle_rights; }
        int get_halfmove() { return halfmove; }
        int get_fullmove() { return fullmove; }
        int get_bot_best_move() { return bot_best_move; }

        
        void set_bitboard(int piece, U64 new_bitboard) { bitboards[piece] = new_bitboard; }
        void set_bitboards(U64 new_bitboards[12]) { memcpy(bitboards, new_bitboards, sizeof(bitboards)); }
        void set_occupancies(U64 new_occupancies[3]) { memcpy(occupancies, new_occupancies, sizeof(occupancies)); }
        void set_side(int new_side) { side = new_side; }
        void set_en_passant_square(int new_en_passant_square) { en_passant_square = new_en_passant_square; }
        void set_castle_rights(int new_castle_rights) { castle_rights = new_castle_rights; }
        void set_halfmove(int new_halfmove) { halfmove = new_halfmove; }
        void set_fullmove(int new_fullmove) { fullmove = new_fullmove; }
    

    private:
        // Board state variables
        U64 bitboards[12]; // 12 bitboards for each piece type (6 for white and 6 for black)
        U64 occupancies[3]; // occupancy bitboards for each color and all pieces
        int side; // side to move
        int en_passant_square = no_sq;
        int castle_rights; 
        int halfmove;
        int fullmove;
        
        // Testing
        int ply = 0; // ply counter for the search algorithm
        long leaf_nodes = 0;
        long perf_captures = 0;
        long perf_enpassant = 0;
        long perf_castlings = 0;
        long perf_promotions = 0;
        long perf_checks = 0;

        float board_evaluation();
        inline float quiescence(float alpha, float beta);
        int bot_best_move;

        // Move generation 
        
        void init_magic_numbers();
        void init_leapers_attacks(); // pawns, knights and king attacks
        void init_sliders_attacks(int bishop); // bishops and rooks attacks (and queens)

        U64 set_occupancy(int index, int bits, U64 attack_mask);

        U64 pawn_attacks[2][64]; // pawns attacks [color][square]
        U64 mask_pawn_attacks(int color, int square);

        U64 knight_attacks[64];
        U64 mask_knight_attacks(int square);

        U64 king_attacks[64];
        U64 mask_king_attacks(int square);

        U64 bishop_attacks[64][512]; // [square][occupancies]
        U64 bishop_masks[64];
        U64 mask_bishop_attacks(int square);
        inline U64 get_bishop_attacks(int square, U64 occupancy) // input occupancy is the board occupancy
        {
            occupancy &= bishop_masks[square]; // See what squares are occupied in the bishop's vision
            occupancy *= bishop_magic_numbers[square]; // The next two steps are the process of retrieving the magic index
            occupancy >>= 64 - bishop_relevant_bits[square]; 

            return bishop_attacks[square][occupancy];
        }

        U64 rook_attacks[64][4096]; // [square][occupancies]
        U64 rook_masks[64];
        U64 mask_rook_attacks(int square);
        inline U64 get_rook_attacks(int square, U64 occupancy) // input occupancy is the board occupancy
        {
            occupancy &= rook_masks[square]; // See what squares are occupied in the bishop's vision
            occupancy *= rook_magic_numbers[square]; // The next two steps are the process of retrieving the magic index
            occupancy >>= 64 - rook_relevant_bits[square]; 

            return rook_attacks[square][occupancy];
        }

        U64 bishop_attacks_on_the_fly(int square, U64 blocks);
        
        U64 rook_attacks_on_the_fly(int square, U64 blocks); 

        inline U64 get_queen_attacks(int square, U64 occupancy)
        {
            return get_bishop_attacks(square, occupancy) | get_rook_attacks(square, occupancy);
        }

        void print_attacked_square(int side);

        inline void generate_alpha_moves(moves* move_list, moves* move_list_alpha)
        {

            move_list->count = 0;
            move_list_alpha->count = 0;

            int source_square, target_square;

            U64 bitboard, attacks;

            for (int piece=P; piece <= k; piece++)
            {
                bitboard = bitboards[piece];
                
                if (side == white) // Special Moves (pawns and kings)
                {
                    if (piece == P)
                    {
                        while (bitboard)
                        {
                            source_square = get_least_significant_bit(bitboard);

                            int source_col = (side == white) ? source_square % 8 : source_square % 8;
                            int source_row = (side == white) ? 7 - source_square / 8 : source_square / 8;

                            target_square = source_square - 8; 

                            // Quiet Pawn Move
                            if (!(target_square < a8) && !get_bit(occupancies[both], target_square))
                            {
                                if (source_square >= a7 && source_square <= h7)
                                {
                                    add_move(move_list_alpha, encode_alpha_move(source_square, 0, 1, 0, 0, 0, 0)); // No underpromotion
                                    add_move(move_list_alpha, encode_alpha_move(source_square, 0, 1, 0, 2, 0, 0)); // Knight underpromotion 
                                    add_move(move_list_alpha, encode_alpha_move(source_square, 0, 1, 0, 5, 0, 0)); // Bishop underpromotion
                                    add_move(move_list_alpha, encode_alpha_move(source_square, 0, 1, 0, 8, 0, 0)); // Rook underpromotion

                                    add_move(move_list, encode_move(source_square, target_square, P, Q, 0, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, P, R, 0, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, P, B, 0, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, P, N, 0, 0, 0, 0));           
                                }
                                else
                                {
                                    add_move(move_list_alpha, encode_alpha_move(source_square, 0, 1, 0, 0, 0, 0)); // Normal pawn push
                                    add_move(move_list, encode_move(source_square, target_square, P, 0, 0, 0, 0, 0));

                                    if (source_square >= a2 && source_square <= h2 && 
                                        !get_bit(occupancies[both], target_square - 8)) // Second rank double push (with no piece in between)
                                    {
                                        add_move(move_list_alpha, encode_alpha_move(source_square, 0, 2, 0, 0, 0, 0)); // Double pawn push                                 
                                        add_move(move_list, encode_move(source_square, target_square - 8, P, 0, 0, 1, 0, 0));                                   
                                    }
                                }
                            }

                            attacks = pawn_attacks[white][source_square] & occupancies[black];

                            while (attacks)
                            {
                                target_square = get_least_significant_bit(attacks);
                                int target_col = target_square % 8;
                                int target_row = 7 - target_square / 8;
                                
                                int direction = (target_col > source_col) ? 1 : 7; // NE or NW
                                int delta_col = target_col - source_col;

                                if (source_square >= a7 && source_square <= h7)
                                {
                                    
                                    add_move(move_list_alpha, encode_alpha_move(source_square, direction, 1, 0, 0, 0, 0));
                                    add_move(move_list_alpha, encode_alpha_move(source_square, direction, 1, 0, 2 - delta_col, 0, 0));
                                    add_move(move_list_alpha, encode_alpha_move(source_square, direction, 1, 0, 5 - delta_col, 0, 0));
                                    add_move(move_list_alpha, encode_alpha_move(source_square, direction, 1, 0, 8 - delta_col, 0, 0));
                                
                                    add_move(move_list, encode_move(source_square, target_square, P, Q, 1, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, P, R, 1, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, P, B, 1, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, P, N, 1, 0, 0, 0));
                                }
                                else
                                {
                                    add_move(move_list_alpha, encode_alpha_move(source_square, direction, 1, 0, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, P, 0, 1, 0, 0, 0));     
                                }
                                
                                clear_bit(attacks, target_square);
                            }
                            if (en_passant_square != no_sq) // The en passant square is not occupied, therefore will not be caught by the logic before
                            {
                                U64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << en_passant_square);
                                if (enpassant_attacks)
                                {
                                    int target_enpassant = get_least_significant_bit(enpassant_attacks);
                                    int enpassant_col = target_enpassant % 8;
                                    int enpassant_row = 7 - target_enpassant / 8;

                                    int direction = (enpassant_col > source_col) ? 1 : 7; // NE or NW

                                    add_move(move_list_alpha, encode_alpha_move(source_square, direction, 1, 0, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_enpassant, P, 0, 1, 0, 1, 0));
                                }
                            }

                            clear_bit(bitboard, source_square);
                        }
                    }

                    if (piece == K) // Castling
                    {
                        if (castle_rights & wk)
                        {
                            if (!get_bit(occupancies[both], f1) && !get_bit(occupancies[both], g1))
                            {
                                if (!is_square_attacked(e1, black) && !is_square_attacked(f1, black))
                                {
                                    add_move(move_list_alpha, encode_alpha_move(e1, 6, 2, 0, 0, 0, 0));
                                    add_move(move_list, encode_move(e1, g1, K, 0, 0, 0, 0, 1));
                                }
                            }  
                        }
                        if (castle_rights & wq)
                        {
                            if (!get_bit(occupancies[both], d1) && !get_bit(occupancies[both], c1) && !get_bit(occupancies[both], b1))
                            {
                                if (!is_square_attacked(e1, black) && !is_square_attacked(d1, black))
                                {
                                    add_move(move_list_alpha, encode_alpha_move(e1, 2, 2, 0, 0, 0, 0));
                                    add_move(move_list, encode_move(e1, c1, K, 0, 0, 0, 0, 1));
                                }
                            }  
                        }
                    }
                }
                else
                {
                    if (piece == p)
                    {
                        while (bitboard)
                        {
                            source_square = get_least_significant_bit(bitboard);
                            int source_col = (side == white) ? source_square % 8 : source_square % 8;
                            int source_row = (side == white) ? 7 - source_square / 8 : source_square / 8;

                            target_square = source_square + 8; // Moves up (not an attack, so not present in attack tables)


                            // Quiet Pawn Move
                            if (!(target_square > h1) && !get_bit(occupancies[both], target_square))
                            {
                                if (source_square >= a2 && source_square <= h2)
                                { // This is exacltly the same as white, because alphazero is always in the prespective of the player
                                    add_move(move_list_alpha, encode_alpha_move(source_square, 0, 1, 0, 0, 0, 1)); // No underpromotion
                                    add_move(move_list_alpha, encode_alpha_move(source_square, 0, 1, 0, 2, 0, 1)); // Knight underpromotion 
                                    add_move(move_list_alpha, encode_alpha_move(source_square, 0, 1, 0, 5, 0, 1)); // Bishop underpromotion
                                    add_move(move_list_alpha, encode_alpha_move(source_square, 0, 1, 0, 8, 0, 1)); // Rook underpromotion

                                    add_move(move_list, encode_move(source_square, target_square, p, q, 0, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, p, n, 0, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, p, b, 0, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, p, r, 0, 0, 0, 0));
                                }
                                else
                                {
                                    add_move(move_list_alpha, encode_alpha_move(source_square, 0, 1, 0, 0, 0, 1)); // Normal pawn push
                                    add_move(move_list, encode_move(source_square, target_square, p, 0, 0, 0, 0, 0));
                                
                                    if (source_square >= a7 && source_square <= h7 && 
                                        !get_bit(occupancies[both], target_square + 8)) // Second rank double push (with no piece in between)
                                    {
                                        add_move(move_list_alpha, encode_alpha_move(source_square, 0, 2, 0, 0, 0, 1)); // Normal pawn push
                                        add_move(move_list, encode_move(source_square, target_square + 8, p, 0, 0, 1, 0, 0));
                                    }
                                }
                            }

                            attacks = pawn_attacks[black][source_square] & occupancies[white];

                            while (attacks)
                            {
                                target_square = get_least_significant_bit(attacks);
                                int target_col = target_square % 8;
                                int target_row = target_square / 8;
                                
                                int direction = (target_col > source_col) ? 1 : 7; // NE or NW
                                int delta_col = target_col - source_col;
                                
                                if (source_square >= a2 && source_square <= h2)
                                {
                                    add_move(move_list_alpha, encode_alpha_move(source_square, direction, 1, 0, 0, 0, 1));
                                    add_move(move_list_alpha, encode_alpha_move(source_square, direction, 1, 0, 2 - delta_col, 0, 1));
                                    add_move(move_list_alpha, encode_alpha_move(source_square, direction, 1, 0, 5 - delta_col, 0, 1));
                                    add_move(move_list_alpha, encode_alpha_move(source_square, direction, 1, 0, 8 - delta_col, 0, 1));
                                
                                    add_move(move_list, encode_move(source_square, target_square, p, q, 1, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, p, n, 1, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, p, b, 1, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, p, r, 1, 0, 0, 0));
                                
                                }
                                else
                                {
                                    add_move(move_list_alpha, encode_alpha_move(source_square, direction, 1, 0, 0, 0, 1));
                                    add_move(move_list, encode_move(source_square, target_square, p, 0, 1, 0, 0, 0));
                                }
                                
                                clear_bit(attacks, target_square);
                            }
                            if (en_passant_square != no_sq) // The en passant square is not occupied, therefore will not be caught by the logic before
                            {
                                U64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << en_passant_square);
                                if (enpassant_attacks)
                                {
                                    int target_enpassant = get_least_significant_bit(enpassant_attacks);
                                    int enpassant_col = target_enpassant % 8;
                                    int enpassant_row = target_enpassant / 8;

                                    int direction = (enpassant_col > source_col) ? 1 : 7; // NE or NW

                                    add_move(move_list_alpha, encode_alpha_move(source_square, direction, 1, 0, 0, 0, 1));
                                    add_move(move_list, encode_move(source_square, target_enpassant, p, 0, 1, 0, 1, 0));
                                }
                            }
                            clear_bit(bitboard, source_square);
                        }
                    }
                    if (piece == k) // Castling
                    {
                        if (castle_rights & bk)
                        {
                            if (!get_bit(occupancies[both], f8) && !get_bit(occupancies[both], g8))
                            {
                                if (!is_square_attacked(e8, white) && !is_square_attacked(f8, white))
                                {
                                    add_move(move_list_alpha, encode_alpha_move(e8, 2, 2, 0, 0, 0, 1));
                                    add_move(move_list, encode_move(e8, g8, k, 0, 0, 0, 0, 1));
                                }
                            }  
                        }
                        if (castle_rights & bq)
                        {
                            if (!get_bit(occupancies[both], d8) && !get_bit(occupancies[both], c8) && !get_bit(occupancies[both], b8))
                            {
                                if (!is_square_attacked(e8, white) && !is_square_attacked(d8, white))
                                {
                                    add_move(move_list_alpha, encode_alpha_move(e8, 6, 2, 0, 0, 0, 1));
                                    add_move(move_list, encode_move(e8, c8, k, 0, 0, 0, 0, 1));
                                }
                            }  
                        }
                    }
                }

                if ((side == white) ? piece == N : piece == n) // Knights
                {
                    while (bitboard)
                    {
                        source_square = get_least_significant_bit(bitboard);
                        int source_col = (side == white) ? source_square % 8 : source_square % 8;
                        int source_row = (side == white) ? 7 - source_square / 8 : source_square / 8;

                        attacks = knight_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                        if (attacks)
                        {
                            while (attacks)
                            {
                                target_square = get_least_significant_bit(attacks);
                                int target_col = (side == white) ? target_square % 8 : target_square % 8;
                                int target_row = (side == white) ? 7 - target_square / 8 : target_square / 8;
                                
                                int row_idx = target_row - source_row + 2;
                                int col_idx = target_col - source_col + 2;

                                add_move(move_list_alpha, encode_alpha_move(source_square, 0, 0, knight_jumps[row_idx][col_idx], 0, 1, side));
                                
                                if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                                {
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                                }
                                else
                                {
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                                }

                                clear_bit(attacks, target_square);
                            }
                        }

                        clear_bit(bitboard, source_square);
                    }
                }

                if ((side == white) ? piece == B : piece == b) // Bishops
                {
                    while (bitboard)
                    {
                        source_square = get_least_significant_bit(bitboard);
                        int source_col = (side == white) ? source_square % 8 :  source_square % 8;
                        int source_row = (side == white) ? 7 - source_square / 8 : source_square / 8;

                        attacks = get_bishop_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                        if (attacks)
                        {
                            while (attacks)
                            {
                                target_square = get_least_significant_bit(attacks);
                                int target_col = (side == white) ? target_square % 8 : target_square % 8;
                                int target_row = (side == white) ? 7 - target_square / 8 : target_square / 8;

                                int delta_row = target_row - source_row;
                                int delta_col = target_col - source_col;
                                
                                int direction;

                                // if (side == white)
                                // {
                                //     // if (delta_row == delta_col) direction = (delta_row > 0) ? 1 : 5; // NE or SW
                                //     // else direction = (delta_row > 0) ? 7 : 3; // NW or SE

                                // }
                                // else 
                                // {
                                //     if (delta_row == delta_col) direction = (delta_row > 0) ? 5 : 1; // NE or SW
                                //     else direction = (delta_row > 0) ? 3 : 7; // NW or SE    
                                // }

                                if (delta_row == delta_col) direction = (delta_row > 0) ? 1 : 5; // NE or SW
                                else direction = (delta_row > 0) ? 7 : 3; // NW or SE
                                
                                int lenght = abs(delta_row);

                                add_move(move_list_alpha, encode_alpha_move(source_square, direction, lenght, 0, 0, 0, side));
                                
                                if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                                {
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                                }
                                else
                                {
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                                }

                                clear_bit(attacks, target_square);
                            }
                        }

                        clear_bit(bitboard, source_square);
                    }
                }
            
                if ((side == white) ? piece == R : piece == r) // Rooks
                {
                    while (bitboard)
                    {
                        source_square = get_least_significant_bit(bitboard);
                        int source_col = (side == white) ? source_square % 8 : source_square % 8;
                        int source_row = (side == white) ? 7 - source_square / 8 : source_square / 8;

                        attacks = get_rook_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                        if (attacks)
                        {
                            while (attacks)
                            {
                                target_square = get_least_significant_bit(attacks);
                                int target_col = (side == white) ? target_square % 8 : target_square % 8;
                                int target_row = (side == white) ? 7 - target_square / 8 : target_square / 8;

                                int delta_row = target_row - source_row;
                                int delta_col = target_col - source_col;

                                int direction;
                                int lenght;

                                if (delta_col == 0)
                                {
                                    // if (side == white) direction = (delta_row > 0) ? 0 : 4; // N or S
                                    // else direction = (delta_row > 0) ? 4 : 0; // Flip for black

                                    direction = (delta_row > 0) ? 0 : 4; // N or S

                                    lenght = std::abs(delta_row);
                                }
                                else
                                {
                                    direction = (delta_col > 0) ? 2 : 6; // E or W
                                    lenght = std::abs(delta_col);
                                }

                                add_move(move_list_alpha, encode_alpha_move(source_square, direction, lenght, 0, 0, 0, side));
                                
                                if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                                {
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                                }
                                else
                                {
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                                }

                                clear_bit(attacks, target_square);
                            }
                        }

                        clear_bit(bitboard, source_square);
                    }
                }

                if ((side == white) ? piece == Q : piece == q) // Queens
                {
                    while (bitboard)
                    {
                        source_square = get_least_significant_bit(bitboard);
                        int source_col = (side == white) ? source_square % 8 : source_square % 8;
                        int source_row = (side == white) ? 7 - source_square / 8 : source_square / 8;

                        attacks = get_queen_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                        if (attacks)
                        {
                            while (attacks)
                            {
                                target_square = get_least_significant_bit(attacks);
                                int target_col = (side == white) ? target_square % 8 : target_square % 8;
                                int target_row = (side == white) ? 7 - target_square / 8 : target_square / 8;

                                int delta_row = target_row - source_row;
                                int delta_col = target_col - source_col;
                                
                                int direction;
                                int lenght;

                                if (delta_col == 0)
                                {
                                    // if (side == white) direction = (delta_row > 0) ? 0 : 4; // N or S
                                    // else direction = (delta_row > 0) ? 4 : 0; // Flip for black

                                    direction = (delta_row > 0) ? 0 : 4; // N or S

                                    lenght = std::abs(delta_row);
                                }
                                else if (delta_row == 0)
                                {
                                    direction = (delta_col > 0) ? 2 : 6; // E or W
                                    lenght = std::abs(delta_col);
                                }
                                else if (delta_row == delta_col)
                                {
                                    // if (side == white) direction = (delta_row > 0) ? 1 : 5; // NE or SW
                                    // else direction = (delta_row > 0) ? 5 : 1; // Flip for black

                                    direction = (delta_row > 0) ? 1 : 5; // NE or SW

                                    lenght = std::abs(delta_col);
                                }
                                else
                                {
                                    // if (side == white) direction = (delta_row > 0) ? 7 : 3; // NW or SE
                                    // else direction = (delta_row > 0) ? 3 : 7; // Flip for black

                                    direction = (delta_row > 0) ? 7 : 3; // NW or SE

                                    lenght = std::abs(delta_col);
                                }

                                add_move(move_list_alpha, encode_alpha_move(source_square, direction, lenght, 0, 0, 0, side));
                                
                                if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                                {
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                                }
                                else
                                {
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                                }

                                clear_bit(attacks, target_square);
                            }
                        }

                        clear_bit(bitboard, source_square);
                    }
                }
            
                if ((side == white) ? piece == K : piece == k) // King
                {
                    while (bitboard)
                    {
                        source_square = get_least_significant_bit(bitboard);
                        int source_col = (side == white) ? source_square % 8 :  source_square % 8;
                        int source_row = (side == white) ? 7 - source_square / 8 : source_square / 8;

                        attacks = king_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                        if (attacks)
                        {
                            while (attacks)
                            {
                                target_square = get_least_significant_bit(attacks);
                                int target_col = (side == white) ? target_square % 8 : target_square % 8;
                                int target_row = (side == white) ? 7 - target_square / 8 : target_square / 8;

                                int delta_row = target_row - source_row;
                                int delta_col = target_col - source_col;
                                
                                int direction;

                                if (delta_col == 0)
                                {
                                    // if (side == white) direction = (delta_row > 0) ? 0 : 4; // N or S
                                    // else direction = (delta_row > 0) ? 4 : 0; // Flip for black

                                    direction = (delta_row > 0) ? 0 : 4; // N or S
                                }
                                else if (delta_row == 0)
                                {
                                    direction = (delta_col > 0) ? 2 : 6; // E or W
                                }
                                else if (delta_row == delta_col)
                                {
                                    // if (side == white) direction = (delta_row > 0) ? 1 : 5; // NE or SW
                                    // else direction = (delta_row > 0) ? 5 : 1; // Flip for black

                                    direction = (delta_row > 0) ? 1 : 5; // NE or SW
                                }
                                else
                                {
                                    // if (side == white) direction = (delta_row > 0) ? 7 : 3; // NW or SE
                                    // else direction = (delta_row > 0) ? 3 : 7; // Flip for black

                                    direction = (delta_row > 0) ? 7 : 3; // NW or SE
                                }
                            
                                add_move(move_list_alpha, encode_alpha_move(source_square, direction, 1, 0, 0, 0, side));
                                
                                if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                                {
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                                }
                                else
                                {
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                                }

                                clear_bit(attacks, target_square);
                            }
                        }

                        clear_bit(bitboard, source_square);
                    }
                }
            }

        }

        inline int is_square_attacked(int square, int side) // Is the given side attacking the given square
        {

            // Strange way to use attacks. We see the enemy attack to check if we can attack it.
            if ((side == white) && (pawn_attacks[black][square] & bitboards[P])) return 1;
            if ((side == black) && (pawn_attacks[white][square] & bitboards[p])) return 1;

            if (knight_attacks[square] & ((side == white) ? bitboards[N] : bitboards[n])) return 1;

            if (get_bishop_attacks(square, occupancies[both]) & (side == white ? bitboards[B] : bitboards[b])) return 1;
            
            if (get_rook_attacks(square, occupancies[both]) & (side == white ? bitboards[R] : bitboards[r])) return 1;
            
            if (get_queen_attacks(square, occupancies[both]) & (side == white ? bitboards[Q] : bitboards[q])) return 1;
            
            if (king_attacks[square] & (side == white ? bitboards[K] : bitboards[k])) return 1;

            return 0;
        }

        inline int make_move(int move, int move_flag)
        {
            if (move_flag == all_moves)
            {
                copy_board();
                int source_square = get_move_source(move);
                int target_square = get_move_target(move);
                int piece = get_move_piece(move);
                int capture = get_move_capture(move);
                int promoted_piece = get_move_promoted(move);
                int enpassant = get_move_enpassant(move);
                int castle = get_move_castling(move);
                int double_push = get_move_double(move);        

                int opponent = (side == white) ? black : white;

                // move the piece
                clear_bit(bitboards[piece], source_square);
                set_bit(bitboards[piece], target_square);

                clear_bit(occupancies[side], source_square);
                set_bit(occupancies[side], target_square);

                halfmove++;
                if (side == black) fullmove++;
                if (piece == P || piece == p) halfmove = 0;


                // captures
                if ((bool)capture)
                {
                    halfmove = 0;
                    int start_piece, end_piece;

                    if (side == white)
                    {
                        start_piece = p;
                        end_piece = k;
                    }
                    else
                    {
                        start_piece = P;
                        end_piece = K;    
                    }

                    for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++)
                    {
                        if (get_bit(bitboards[bb_piece], target_square))
                        {
                            clear_bit(bitboards[bb_piece], target_square);
                            clear_bit(occupancies[opponent], target_square);
                            break;
                        }
                    }
                }

                if (promoted_piece)
                {
                    clear_bit(bitboards[(side == white) ? P : p], target_square);

                    set_bit(bitboards[promoted_piece], target_square);
                }

                if ((bool)enpassant)
                {
                    if (side == white)
                    {
                        clear_bit(bitboards[p], target_square + 8);
                        clear_bit(occupancies[opponent], target_square + 8);
                    }
                    else
                    {
                        clear_bit(bitboards[P], target_square - 8);
                        clear_bit(occupancies[opponent], target_square - 8);
                    }
                }

                if ((bool)double_push)
                {
                    en_passant_square = ((side == white) ?  target_square + 8 : target_square - 8);
                } else en_passant_square = no_sq;

                if (castle)
                {
                    switch (target_square)
                    {
                    case g1:
                        clear_bit(bitboards[R], h1);
                        set_bit(bitboards[R], f1);
                        clear_bit(occupancies[side], h1);
                        set_bit(occupancies[side], f1);
                        break;
                    case c1:
                        clear_bit(bitboards[R], a1);
                        set_bit(bitboards[R], d1);
                        clear_bit(occupancies[side], a1);
                        set_bit(occupancies[side], d1);
                        break;
                    case g8:
                        clear_bit(bitboards[r], h8);
                        set_bit(bitboards[r], f8);
                        clear_bit(occupancies[side], h8);
                        set_bit(occupancies[side], f8);
                        break;
                    case c8:
                        clear_bit(bitboards[r], a8);
                        set_bit(bitboards[r], d8);
                        clear_bit(occupancies[side], a8);
                        set_bit(occupancies[side], d8);
                        break;
                    
                    default:
                        break;
                    }
                }

                castle_rights &= castling_rights_board[source_square]; // If pieces move
                castle_rights &= castling_rights_board[target_square]; // If rooks are captured

                // Update occupancies
                occupancies[both] = occupancies[side] | occupancies[opponent];

                // memset(occupancies, 0x0ULL, sizeof(occupancies));
                // for (int bb_piece = P; bb_piece <= K; bb_piece++)
                //     occupancies[white] |= bitboards[bb_piece];
                
                // for (int bb_piece = p; bb_piece <= k; bb_piece++)
                //     occupancies[black] |= bitboards[bb_piece];
                
                // occupancies[both] |= occupancies[white];
                // occupancies[both] |= occupancies[black];


                // Change sides with XOR operator
                side ^= 1;

                if (is_square_attacked(get_least_significant_bit((side == white) ? bitboards[k] : bitboards[K]) , side))
                { // Illegal move
                    restore_board();
                    return 0;
                }
                else
                {
                    return 1;
                }
            }
            else
            {
                if ((bool)get_move_capture(move))
                {   
                    return make_move(move, all_moves);
                }
                else
                {
                    return 0;
                }
            }
            return 0;
        }

        inline void generate_moves(moves* move_list) // provides the pseudo-legals moves
        {
            move_list->count = 0;

            int source_square, target_square;

            U64 bitboard, attacks;

            for (int piece=P; piece <= k; piece++)
            {
                bitboard = bitboards[piece];
                
                if (side == white) // Special Moves (pawns and kings)
                {
                    if (piece == P)
                    {
                        while (bitboard)
                        {
                            source_square = get_least_significant_bit(bitboard);

                            target_square = source_square - 8; // Moves up (not an attack, so not present in attack tables)

                            // Quiet Pawn Move
                            if (!(target_square < a8) && !get_bit(occupancies[both], target_square))
                            {
                                if (source_square >= a7 && source_square <= h7)
                                {
                                    add_move(move_list, encode_move(source_square, target_square, P, Q, 0, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, P, R, 0, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, P, B, 0, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, P, N, 0, 0, 0, 0));
                                }
                                else
                                {
                                    add_move(move_list, encode_move(source_square, target_square, P, 0, 0, 0, 0, 0));

                                    if (source_square >= a2 && source_square <= h2 && 
                                        !get_bit(occupancies[both], target_square - 8)) // Second rank double push (with no piece in between)
                                    {
                                        add_move(move_list, encode_move(source_square, target_square - 8, P, 0, 0, 1, 0, 0));                                   
                                    }
                                }
                            }

                            attacks = pawn_attacks[white][source_square] & occupancies[black];

                            while (attacks)
                            {
                                target_square = get_least_significant_bit(attacks);
                                
                                if (source_square >= a7 && source_square <= h7)
                                {
                                    add_move(move_list, encode_move(source_square, target_square, P, Q, 1, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, P, R, 1, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, P, B, 1, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, P, N, 1, 0, 0, 0));
                                }
                                else
                                {
                                    add_move(move_list, encode_move(source_square, target_square, P, 0, 1, 0, 0, 0));
                                }
                                
                                clear_bit(attacks, target_square);
                            }
                            if (en_passant_square != no_sq) // The en passant square is not occupied, therefore will not be caught by the logic before
                            {
                                U64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << en_passant_square);
                                if (enpassant_attacks)
                                {
                                    int target_enpassant = get_least_significant_bit(enpassant_attacks);
                                    add_move(move_list, encode_move(source_square, target_enpassant, P, 0, 1, 0, 1, 0));
                                }
                            }

                            clear_bit(bitboard, source_square);
                        }
                    }

                    if (piece == K) // Castling
                    {
                        if (castle_rights & wk)
                        {
                            if (!get_bit(occupancies[both], f1) && !get_bit(occupancies[both], g1))
                            {
                                if (!is_square_attacked(e1, black) && !is_square_attacked(f1, black))
                                {
                                    add_move(move_list, encode_move(e1, g1, K, 0, 0, 0, 0, 1));
                                }
                            }  
                        }
                        if (castle_rights & wq)
                        {
                            if (!get_bit(occupancies[both], d1) && !get_bit(occupancies[both], c1) && !get_bit(occupancies[both], b1))
                            {
                                if (!is_square_attacked(e1, black) && !is_square_attacked(d1, black))
                                {
                                    add_move(move_list, encode_move(e1, c1, K, 0, 0, 0, 0, 1));
                                }
                            }  
                        }
                    }
                }
                else
                {
                    if (piece == p)
                    {
                        while (bitboard)
                        {
                            source_square = get_least_significant_bit(bitboard);

                            target_square = source_square + 8; // Moves up (not an attack, so not present in attack tables)

                            // Quiet Pawn Move
                            if (!(target_square > h1) && !get_bit(occupancies[both], target_square))
                            {
                                if (source_square >= a2 && source_square <= h2)
                                {
                                    add_move(move_list, encode_move(source_square, target_square, p, q, 0, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, p, n, 0, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, p, b, 0, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, p, r, 0, 0, 0, 0));
                                }
                                else
                                {
                                    add_move(move_list, encode_move(source_square, target_square, p, 0, 0, 0, 0, 0));
                                
                                    if (source_square >= a7 && source_square <= h7 && 
                                        !get_bit(occupancies[both], target_square + 8)) // Second rank double push (with no piece in between)
                                    {
                                        add_move(move_list, encode_move(source_square, target_square + 8, p, 0, 0, 1, 0, 0));
                                    }
                                }
                            }

                            attacks = pawn_attacks[black][source_square] & occupancies[white];

                            while (attacks)
                            {
                                target_square = get_least_significant_bit(attacks);
                                
                                if (source_square >= a2 && source_square <= h2)
                                {
                                    add_move(move_list, encode_move(source_square, target_square, p, q, 1, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, p, n, 1, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, p, b, 1, 0, 0, 0));
                                    add_move(move_list, encode_move(source_square, target_square, p, r, 1, 0, 0, 0));
                                }
                                else
                                {
                                    add_move(move_list, encode_move(source_square, target_square, p, 0, 1, 0, 0, 0));
                                }
                                
                                clear_bit(attacks, target_square);
                            }
                            if (en_passant_square != no_sq) // The en passant square is not occupied, therefore will not be caught by the logic before
                            {
                                U64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << en_passant_square);
                                if (enpassant_attacks)
                                {
                                    int target_enpassant = get_least_significant_bit(enpassant_attacks);
                                    add_move(move_list, encode_move(source_square, target_enpassant, p, 0, 1, 0, 1, 0));
                                }
                            }
                            clear_bit(bitboard, source_square);
                        }
                    }
                    if (piece == k) // Castling
                    {
                        if (castle_rights & bk)
                        {
                            if (!get_bit(occupancies[both], f8) && !get_bit(occupancies[both], g8))
                            {
                                if (!is_square_attacked(e8, white) && !is_square_attacked(f8, white))
                                {
                                    add_move(move_list, encode_move(e8, g8, k, 0, 0, 0, 0, 1));
                                }
                            }  
                        }
                        if (castle_rights & bq)
                        {
                            if (!get_bit(occupancies[both], d8) && !get_bit(occupancies[both], c8) && !get_bit(occupancies[both], b8))
                            {
                                if (!is_square_attacked(e8, white) && !is_square_attacked(d8, white))
                                {
                                    add_move(move_list, encode_move(e8, c8, k, 0, 0, 0, 0, 1));
                                }
                            }  
                        }
                    }
                }

                if ((side == white) ? piece == N : piece == n) // Knights
                {
                    while (bitboard)
                    {
                        source_square = get_least_significant_bit(bitboard);

                        attacks = knight_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                        if (attacks)
                        {
                            while (attacks)
                            {
                                target_square = get_least_significant_bit(attacks);
                                
                                if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                                {
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                                }
                                else
                                {
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                                }
                                
                                clear_bit(attacks, target_square);
                            }
                        }

                        clear_bit(bitboard, source_square);
                    }
                }

                if ((side == white) ? piece == B : piece == b) // Bishops
                {
                    while (bitboard)
                    {
                        source_square = get_least_significant_bit(bitboard);

                        attacks = get_bishop_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                        if (attacks)
                        {
                            while (attacks)
                            {
                                target_square = get_least_significant_bit(attacks);
                                
                                if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                                {
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                                }
                                else
                                {
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                                }
                                
                                clear_bit(attacks, target_square);
                            }
                        }

                        clear_bit(bitboard, source_square);
                    }
                }
            
                if ((side == white) ? piece == R : piece == r) // Rooks
                {
                    while (bitboard)
                    {
                        source_square = get_least_significant_bit(bitboard);

                        attacks = get_rook_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                        if (attacks)
                        {
                            while (attacks)
                            {
                                target_square = get_least_significant_bit(attacks);
                                
                                if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                                {
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                                }
                                else
                                {
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                                }
                                
                                clear_bit(attacks, target_square);
                            }
                        }

                        clear_bit(bitboard, source_square);
                    }
                }

                if ((side == white) ? piece == Q : piece == q) // Queens
                {
                    while (bitboard)
                    {
                        source_square = get_least_significant_bit(bitboard);

                        attacks = get_queen_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                        if (attacks)
                        {
                            while (attacks)
                            {
                                target_square = get_least_significant_bit(attacks);
                                
                                if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                                {
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                                }
                                else
                                {
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                                }
                                
                                clear_bit(attacks, target_square);
                            }
                        }

                        clear_bit(bitboard, source_square);
                    }
                }
            
                if ((side == white) ? piece == K : piece == k) // King
                {
                    while (bitboard)
                    {
                        source_square = get_least_significant_bit(bitboard);

                        attacks = king_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                        if (attacks)
                        {
                            while (attacks)
                            {
                                target_square = get_least_significant_bit(attacks);
                                
                                if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                                {
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                                }
                                else
                                {
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                                }
                                
                                clear_bit(attacks, target_square);
                            }
                        }

                        clear_bit(bitboard, source_square);
                    }
                }
            }
        }   


        enum colors{
            white, black, both
        };

        enum slides{
            rook, bishop
        };

        enum pieces 
        {
            P, N, B, R, Q, K, p, n, b, r, q, k,
        };

        int board_conversion[12] = {
            1, 2, 3, 4, 5, 6, -1, -2, -3, -4, -5, -6
        };

        int piece_values[12] = {
            1, 3, 3, 5, 9, 200, -1, -3, -3, -5, -9, -200
        };

        float piece_positional_value[64] = 
        {
            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.0,
            1.0, 1.1, 1.2, 1.2, 1.2, 1.2, 1.1, 1.0,
            1.0, 1.1, 1.2, 1.2, 1.2, 1.2, 1.1, 1.0,
            1.0, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        };

        float king_safety_value[64] = 
        {
            1.5, 2.0, 2.0, 1.0, 1.0, 2.0, 2.0, 1.5,
            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
            0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5,
            0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5,
            0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5,
            0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5,
            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
            1.5, 2.0, 2.0, 1.0, 1.0, 2.0, 2.0, 1.5,
        };



        enum move_types 
        {
            all_moves, only_captures
        };


        enum board_square{
            a8, b8, c8, d8, e8, f8, g8, h8,
            a7, b7, c7, d7, e7, f7, g7, h7,
            a6, b6, c6, d6, e6, f6, g6, h6,
            a5, b5, c5, d5, e5, f5, g5, h5,
            a4, b4, c4, d4, e4, f4, g4, h4,
            a3, b3, c3, d3, e3, f3, g3, h3,
            a2, b2, c2, d2, e2, f2, g2, h2,
            a1, b1, c1, d1, e1, f1, g1, h1, no_sq
        }; 

        enum 
        {
            wk = 1, wq = 2, bk = 4, bq = 8, // 0001, 0010, 0100, 1000 in decimal. Summing all give 1111 
        };


        /*
                                    Castling Rights     move update         binary      decimal
            no king and rook move:      1111        &      1111       =      1111          15

                 white king moved:      1111        &      1100       =      1100          12
          white king's rook moved:      1111        &      1110       =      1110          14
         white queen's rook moved:      1111        &      1101       =      1101          13
        
                 black king moved:      1111        &      0011       =      0011           3
          black king's rook moved:      1111        &      1011       =      1011          11
         black queen's rook moved:      1111        &      0111       =      0111           7
        */

        int castling_rights_board[64] = {
             7, 15, 15, 15,  3, 15, 15, 11,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            13, 15, 15, 15, 12, 15, 15, 14,
        };

        unsigned int state = 1804289383;
        unsigned int get_random_U32_number();
        U64 get_random_U64_number();
        U64 generate_magic_number();
        U64 find_magic_number(int square, int occupancy_bits, int bishop);

        /* Move enconding 
                    
                    Binary Representation                            Hexadecimal Constants

        0000 0000 0000 0000 0011 1111   source square    [64]               0x3f   
        0000 0000 0000 1111 1100 0000   target square    [64]               0xfc0
        0000 0000 1111 0000 0000 0000   piece            [12]               0xf000
        0000 1111 0000 0000 0000 0000   promoted piece   [12]               0xf0000
        0001 0000 0000 0000 0000 0000   capture flag     [ 1]               0x100000
        0010 0000 0000 0000 0000 0000   double push flag [ 1]               0x200000
        0100 0000 0000 0000 0000 0000   en passant flag  [ 1]               0x400000
        1000 0000 0000 0000 0000 0000   castling flag    [ 1]               0x800000
        
        */

        /* Alpha Move enconding 
                    
                    Binary Representation                            Hexadecimal Constants

        0000 0000 0000 0000 0011 1111   source square    [64]               0x3f   
        0000 0000 0000 0001 1100 0000   direction        [ 8]               0x1c0
        0000 0000 0000 1110 0000 0000   length           [ 7]               0xE00
        0000 0000 0111 0000 0000 0000   knight moves     [ 8]               0x7000
        0000 1111 1000 0000 0000 0000   underpromotions  [ 9]               0xF8000
        0001 0000 0000 0000 0000 0000   piece is knight  [ 1]               0x100000
        0010 0000 0000 0000 0000 0000   side             [ 1]               0x200000

        Direction = {N, NE, E, SE, S, SW, W, NW}
        Lenght = {1, 2, 3, 4, 5, 6, 7}
        Knight moves = { Counter Clockwise beginning from top-left 
         
            { 2, -1}, 0 -> [4][1]
            { 2,  1}, 1 -> [4][3]
            { 1,  2}, 2 -> [3][4]
            {-1,  2}, 3 -> [1][4]
            {-2,  1}, 4 -> [0][3]
            {-2, -1}, 5 -> [0][1]
            {-1, -2}, 6 -> [1][0]
            { 1, -2}  7 -> [3][0]
        }

        underpromotions = {No, N_NE, N_N, N_NW, B_NE, B_N, B_NW, R_NE, R_N, R_NW}
                            0    1    2     3    4     5    6     7      8    9
        */

        int knight_jumps[5][5] =
        {
            {-1,  5, -1,  4, -1},
            { 6, -1, -1, -1,  3},
            {-1, -1, -1, -1, -1},
            { 7, -1, -1, -1,  2},
            {-1,  0, -1,  1, -1},
        };


        // Board Visualization Functions
        char ascii_pieces[12] = {'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k'};
        const char *unicode_pieces[12] = {"♟", "♞", "♝", "♜", "♛", "♚", "♙", "♘", "♗", "♖", "♕", "♔"};
        std::unordered_map<char, int> char_pieces = {
            {'P', P}, {'N', N}, {'B', B}, {'R', R}, {'Q', Q}, {'K', K},
            {'p', p}, {'n', n}, {'b', b}, {'r', r}, {'q', q}, {'k', k}
        };
        std::unordered_map<int, char> promoted_pieces = {
            {N, 'n'}, {B, 'b'}, {R, 'r'}, {Q, 'q'},
            {n, 'n'}, {b, 'b'}, {r, 'r'}, {q, 'q'}, {0, ' '}
        };

        const U64 not_a_file = 18374403900871474942ULL;
        /* Not "a" file mask
        8  0 1 1 1 1 1 1 1 
        7  0 1 1 1 1 1 1 1 
        6  0 1 1 1 1 1 1 1 
        5  0 1 1 1 1 1 1 1 
        4  0 1 1 1 1 1 1 1 
        3  0 1 1 1 1 1 1 1 
        2  0 1 1 1 1 1 1 1 
        1  0 1 1 1 1 1 1 1 
        a b c d e f g h
        */        

       const U64 not_h_file = 9187201950435737471ULL;
       /* Not "h" file mask
        8  1 1 1 1 1 1 1 0 
        7  1 1 1 1 1 1 1 0 
        6  1 1 1 1 1 1 1 0 
        5  1 1 1 1 1 1 1 0 
        4  1 1 1 1 1 1 1 0 
        3  1 1 1 1 1 1 1 0 
        2  1 1 1 1 1 1 1 0 
        1  1 1 1 1 1 1 1 0 
        a b c d e f g h
       */

        const U64 not_hg_file = 4557430888798830399ULL;
        /* Not "h" and "g" file mask
        8  1 1 1 1 1 1 0 0 
        7  1 1 1 1 1 1 0 0 
        6  1 1 1 1 1 1 0 0 
        5  1 1 1 1 1 1 0 0 
        4  1 1 1 1 1 1 0 0 
        3  1 1 1 1 1 1 0 0 
        2  1 1 1 1 1 1 0 0 
        1  1 1 1 1 1 1 0 0 
        a b c d e f g h
        */

        const U64 not_ab_file = 18229723555195321596ULL;
        /* Not "a" and "b" file mask
        8  0 0 1 1 1 1 1 1 
        7  0 0 1 1 1 1 1 1 
        6  0 0 1 1 1 1 1 1 
        5  0 0 1 1 1 1 1 1 
        4  0 0 1 1 1 1 1 1 
        3  0 0 1 1 1 1 1 1 
        2  0 0 1 1 1 1 1 1 
        1  0 0 1 1 1 1 1 1 
        a b c d e f g h
        */

        const U64 rook_magic_numbers[64] = {
            0x8a80104000800020ULL,
            0x140002000100040ULL,
            0x2801880a0017001ULL,
            0x100081001000420ULL,
            0x200020010080420ULL,
            0x3001c0002010008ULL,
            0x8480008002000100ULL,
            0x2080088004402900ULL,
            0x800098204000ULL,
            0x2024401000200040ULL,
            0x100802000801000ULL,
            0x120800800801000ULL,
            0x208808088000400ULL,
            0x2802200800400ULL,
            0x2200800100020080ULL,
            0x801000060821100ULL,
            0x80044006422000ULL,
            0x100808020004000ULL,
            0x12108a0010204200ULL,
            0x140848010000802ULL,
            0x481828014002800ULL,
            0x8094004002004100ULL,
            0x4010040010010802ULL,
            0x20008806104ULL,
            0x100400080208000ULL,
            0x2040002120081000ULL,
            0x21200680100081ULL,
            0x20100080080080ULL,
            0x2000a00200410ULL,
            0x20080800400ULL,
            0x80088400100102ULL,
            0x80004600042881ULL,
            0x4040008040800020ULL,
            0x440003000200801ULL,
            0x4200011004500ULL,
            0x188020010100100ULL,
            0x14800401802800ULL,
            0x2080040080800200ULL,
            0x124080204001001ULL,
            0x200046502000484ULL,
            0x480400080088020ULL,
            0x1000422010034000ULL,
            0x30200100110040ULL,
            0x100021010009ULL,
            0x2002080100110004ULL,
            0x202008004008002ULL,
            0x20020004010100ULL,
            0x2048440040820001ULL,
            0x101002200408200ULL,
            0x40802000401080ULL,
            0x4008142004410100ULL,
            0x2060820c0120200ULL,
            0x1001004080100ULL,
            0x20c020080040080ULL,
            0x2935610830022400ULL,
            0x44440041009200ULL,
            0x280001040802101ULL,
            0x2100190040002085ULL,
            0x80c0084100102001ULL,
            0x4024081001000421ULL,
            0x20030a0244872ULL,
            0x12001008414402ULL,
            0x2006104900a0804ULL,
            0x1004081002402ULL,
        };

        const U64 bishop_magic_numbers[64] = {
            0x40040844404084ULL,
            0x2004208a004208ULL,
            0x10190041080202ULL,
            0x108060845042010ULL,
            0x581104180800210ULL,
            0x2112080446200010ULL,
            0x1080820820060210ULL,
            0x3c0808410220200ULL,
            0x4050404440404ULL,
            0x21001420088ULL,
            0x24d0080801082102ULL,
            0x1020a0a020400ULL,
            0x40308200402ULL,
            0x4011002100800ULL,
            0x401484104104005ULL,
            0x801010402020200ULL,
            0x400210c3880100ULL,
            0x404022024108200ULL,
            0x810018200204102ULL,
            0x4002801a02003ULL,
            0x85040820080400ULL,
            0x810102c808880400ULL,
            0xe900410884800ULL,
            0x8002020480840102ULL,
            0x220200865090201ULL,
            0x2010100a02021202ULL,
            0x152048408022401ULL,
            0x20080002081110ULL,
            0x4001001021004000ULL,
            0x800040400a011002ULL,
            0xe4004081011002ULL,
            0x1c004001012080ULL,
            0x8004200962a00220ULL,
            0x8422100208500202ULL,
            0x2000402200300c08ULL,
            0x8646020080080080ULL,
            0x80020a0200100808ULL,
            0x2010004880111000ULL,
            0x623000a080011400ULL,
            0x42008c0340209202ULL,
            0x209188240001000ULL,
            0x400408a884001800ULL,
            0x110400a6080400ULL,
            0x1840060a44020800ULL,
            0x90080104000041ULL,
            0x201011000808101ULL,
            0x1a2208080504f080ULL,
            0x8012020600211212ULL,
            0x500861011240000ULL,
            0x180806108200800ULL,
            0x4000020e01040044ULL,
            0x300000261044000aULL,
            0x802241102020002ULL,
            0x20906061210001ULL,
            0x5a84841004010310ULL,
            0x4010801011c04ULL,
            0xa010109502200ULL,
            0x4a02012000ULL,
            0x500201010098b028ULL,
            0x8040002811040900ULL,
            0x28000010020204ULL,
            0x6000020202d0240ULL,
            0x8918844842082200ULL,
            0x4010011029020020ULL,
        };

        void test_bitboard();
        
};

