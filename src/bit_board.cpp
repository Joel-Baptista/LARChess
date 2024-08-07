#include <iostream>
#include <string>
#include <string.h>
#include "bit_board.h"
#include <stdlib.h> 

const char *square_to_coords[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};

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

BitBoard::BitBoard(){
    std::cout << "BitBoard constructor" << std::endl;
    init_leapers_attacks();
    // init_magic_numbers(); // This is a very time consuming process, the magic numbers are already hardcoded
    test_bitboard();
}

BitBoard::~BitBoard(){
    std::cout << "BitBoard destructor" << std::endl;
}

void BitBoard::test_bitboard()
{   
    for (int square = 0; square < 64; square++)
    {
        printf(" 0x%llxULL\n", rook_magic_numbers[square]);
    }

    std::cout << "------------------------------------" << std::endl;

    for (int square = 0; square < 64; square++)
    {
        printf(" 0x%llxULL\n", bishop_magic_numbers[square]);
    }
}

U64 BitBoard::find_magic_number(int square, int occupancy_bits, int bishop)
{
    U64 occupancies[4096];
    U64 attacks[4096];
    U64 used_attacks[4096];

    U64 attack_mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);

    int occupancy_indexes = 1 << occupancy_bits;

    for (int index = 0; index < occupancy_indexes; index++)
    {
        occupancies[index] = set_occupancy(index, occupancy_bits, attack_mask);
        /* As I understand it, the reason we did not use the edge squares in the attack mask is because it is irrelevant
        The attack mask is used to put blocking pieces in the squares. The attack board will see the edge square regardless if there is a piece there
        or not. However, not using the edge square greatly reduces the number of combinational occupancies */
        attacks[index] = bishop ? bishop_attacks_on_the_fly(square, occupancies[index]) : rook_attacks_on_the_fly(square, occupancies[index]);
    }

    for (int random_count = 0; random_count < 100000000; random_count++)
    {
        U64 magic_number = generate_magic_number();

        // Skip inappropriate magic numbers
        if (count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6) continue; 

        memset(used_attacks, 0ULL, sizeof(used_attacks));

        int index, fail;

        for (index = 0, fail = 0; !fail && index < occupancy_indexes; index++)
        {
            int magic_index = (int)((occupancies[index] * magic_number) >> (64 - occupancy_bits));
            if (used_attacks[magic_index] == 0ULL)
            {
                used_attacks[magic_index] = attacks[index];
            }
            else
            {
                fail = 1;
            }
        }
        if (!fail) return magic_number;
    }
    std::cout << "Magic number not found" << std::endl;
    return 0ULL;
}

U64 BitBoard::set_occupancy(int index, int bits, U64 attack_mask)
{ // This creates every possible combination of other pieces occupying the board in the direction of the sliding piece
    U64 occupancy = 0x0ULL;

    for (int cout = 0; cout < bits; cout++)
    {
        int square = get_least_significant_bit(attack_mask);
        clear_bit(attack_mask, square);
        if (index & (1 << cout)) set_bit(occupancy, square);
    }

    return occupancy;
}


U64 BitBoard::mask_pawn_attacks(int color, int square)
{
    U64 attacks = 0x0ULL;
    U64 bitboard = 0x0ULL;

    set_bit(bitboard, square);
    if (!color) // white
    {
        if (((bitboard >> 7) & not_a_file)) attacks |= (bitboard >> 7); 
        if (((bitboard >> 9) & not_h_file)) attacks |= (bitboard >> 9);
    } 
    else // black
    {
        if (((bitboard << 7) & not_h_file)) attacks |= (bitboard << 7); 
        if (((bitboard << 9) & not_a_file)) attacks |= (bitboard << 9);
        
    }   

    return attacks;

}

U64 BitBoard::mask_king_attacks(int square)
{
    U64 attacks = 0x0ULL;
    U64 bitboard = 0x0ULL;

    set_bit(bitboard, square);

    if (bitboard >> 8) attacks |= (bitboard >> 8);
    if ((bitboard >> 9) & not_h_file) attacks |= (bitboard >> 9);
    if ((bitboard >> 7) & not_a_file) attacks |= (bitboard >> 7);
    if ((bitboard >> 1) & not_h_file) attacks |= (bitboard >> 1);
    
    if (bitboard << 8) attacks |= (bitboard << 8);
    if ((bitboard << 9) & not_a_file) attacks |= (bitboard << 9);
    if ((bitboard << 7) & not_h_file) attacks |= (bitboard << 7);
    if ((bitboard << 1) & not_a_file) attacks |= (bitboard << 1);

    return attacks;

}

U64 BitBoard::mask_knight_attacks(int square)
{
    U64 attacks = 0x0ULL;
    U64 bitboard = 0x0ULL;

    set_bit(bitboard, square);

    if (((bitboard >> 17) & not_h_file)) attacks |= (bitboard >> 17);
    if (((bitboard >> 15) & not_a_file)) attacks |= (bitboard >> 15);
    if (((bitboard >> 10) & not_hg_file)) attacks |= (bitboard >> 10);
    if (((bitboard >> 6) & not_ab_file)) attacks |= (bitboard >> 6);
    
    if (((bitboard << 17) & not_a_file)) attacks |= (bitboard << 17);
    if (((bitboard << 15) & not_h_file)) attacks |= (bitboard << 15);
    if (((bitboard << 10) & not_ab_file)) attacks |= (bitboard <<  10);
    if (((bitboard << 6) & not_hg_file)) attacks |= (bitboard << 6);

    return attacks;

}

U64 BitBoard::mask_bishop_attacks(int square)
{
    U64 attacks = 0x0ULL;

    // init rank and file
    int r, f;  

    // init target rank and file
    int tr = square / 8;
    int tf = square % 8;

    for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++) attacks |= (1ULL << (r * 8 + f));
    for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++) attacks |= (1ULL << (r * 8 + f));
    for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--) attacks |= (1ULL << (r * 8 + f));
    for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--) attacks |= (1ULL << (r * 8 + f));

    return attacks;

}

U64 BitBoard::bishop_attacks_on_the_fly(int square, U64 block)
{
    U64 attacks = 0x0ULL;

    // init rank and file
    int r, f;  

    // init target rank and file
    int tr = square / 8;
    int tf = square % 8;

    for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f) & block)) break;
    }
    for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f) & block)) break;
    }
    for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f) & block)) break;
    }
    for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f) & block)) break;
    }


    return attacks;

}

U64 BitBoard::mask_rook_attacks(int square)
{
    U64 attacks = 0x0ULL;

    // init rank and file
    int r, f;  

    // init target rank and file
    int tr = square / 8;
    int tf = square % 8;

    for (r = tr + 1; r <= 6; r++) attacks |= (1ULL << (r * 8 + tf));
    for (r = tr - 1; r >= 1; r--) attacks |= (1ULL << (r * 8 + tf));
    for (f = tf + 1; f <= 6; f++) attacks |= (1ULL << (tr * 8 + f));
    for (f = tf - 1; f >= 1; f--) attacks |= (1ULL << (tr * 8 + f));

    return attacks;

}

U64 BitBoard::rook_attacks_on_the_fly(int square, U64 blocks)
{
    U64 attacks = 0x0ULL;

    // init rank and file
    int r, f;  

    // init target rank and file
    int tr = square / 8;
    int tf = square % 8;

    for (r = tr + 1; r <= 7; r++)
    {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf) & blocks)) break;
    }

    for (r = tr - 1; r >= 0; r--)
    {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf) & blocks)) break;
    }

    for (f = tf + 1; f <= 7; f++)
    {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f) & blocks)) break;
    }

    for (f = tf - 1; f >= 0; f--)
    {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f) & blocks)) break;
    }

    return attacks;

}

void BitBoard::init_leapers_attacks()
{
    for (int square = 0; square < 64; square++)
    {
        pawn_attacks[white][square] = mask_pawn_attacks(white, square);
        pawn_attacks[black][square] = mask_pawn_attacks(black, square);
    
        knight_attacks[square] = mask_knight_attacks(square);

        king_attacks[square] = mask_king_attacks(square);

        bishop_attacks[square] = mask_bishop_attacks(square);

        rook_attacks[square] = mask_rook_attacks(square);        
    }

}

void BitBoard::print_bitboard(U64 bitboard)
{
    for (int rank=0; rank < 8; rank++)
    {
        for (int file=0; file < 8; file++)
        {
            int square = rank * 8 + file;
            
            if (!file)
            {
                std::cout << 8 - rank << "  ";    
            }

            std::cout << (get_bit(bitboard, square) ? 1 : 0) << " ";
            
        }
        std::cout << std::endl;
    }
    std::cout << "   a b c d e f g h\n" << std::endl;
    std::cout << "    Bitboard: "<< bitboard << std::endl;

}

unsigned int BitBoard::get_random_U32_number()
{ // Xorshift32
    unsigned int x = state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    state = x;

    return state;
}

U64 BitBoard::get_random_U64_number()
{
    U64 n1, n2, n3, n4;

    n1 = (U64)(get_random_U32_number() & 0xFFFF);
    n2 = (U64)(get_random_U32_number() & 0xFFFF);
    n3 = (U64)(get_random_U32_number() & 0xFFFF);
    n4 = (U64)(get_random_U32_number() & 0xFFFF);

    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

U64 BitBoard::generate_magic_number()
{
    return get_random_U64_number() & get_random_U64_number() & get_random_U64_number();
}

/* Maintain for future reference. It causes a compile error because the magic numbers are now consts.
void BitBoard::init_magic_numbers()
{
    for (int square = 0; square < 64; square++)
    {
        rook_magic_numbers[square] = find_magic_number(square, rook_relevant_bits[square], rook);
    }

    for (int square = 0; square < 64; square++)
    {
        bishop_magic_numbers[square] = find_magic_number(square, bishop_relevant_bits[square], bishop);
    }

}
*/