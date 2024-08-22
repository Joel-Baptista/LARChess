#include "bit_board.h"

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