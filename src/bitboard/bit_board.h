#pragma once
#include <unordered_map>
#include <iostream>
#include <string>
#include <string.h>
#include <stdlib.h> 
#include <sys/time.h>
#include <array>

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

class BitBoard {
    public:
        BitBoard();
        ~BitBoard();

        void print_bitboard(U64 bitboard);
        void print_board();
        void parse_fen(const char *fen);
        void print_move(int move);
        void print_move_list(moves* move_list);

        // Normal Getters and Setters

        int get_side() { return side; }
        int get_bot_best_move() { return bot_best_move; }

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
        

    private:
        
        // Board state variables
        U64 bitboards[12]; // 12 bitboards for each piece type (6 for white and 6 for black)
        U64 occupancies[3]; // occupancy bitboards for each color and all pieces
        int side; // side to move
        int en_passant_square = no_sq;
        int castle_rights; 
        int halfmove;
        int fullmove;
        int ply = 0; // ply counter for the search algorithm

        // Testing
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
        inline U64  get_bishop_attacks(int square, U64 occupancy);

        U64 rook_attacks[64][4096]; // [square][occupancies]
        U64 rook_masks[64];
        U64 mask_rook_attacks(int square);
        inline U64 get_rook_attacks(int square, U64 occupancy);

        U64 bishop_attacks_on_the_fly(int square, U64 blocks);
        
        U64 rook_attacks_on_the_fly(int square, U64 blocks); 

        inline U64 get_queen_attacks(int square, U64 occupancy);

        inline int is_square_attacked(int square, int side);
        void print_attacked_square(int side);

        inline void generate_moves(moves* move_list);
        inline int make_move(int move, int move_flag);
        int parse_move(const char* move_string);

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

static inline void add_move(moves *move_list, int move);
int get_time_ms();

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

