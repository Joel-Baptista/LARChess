
#include "bit_board.h"


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
    init_sliders_attacks(bishop);
    init_sliders_attacks(rook);
    // init_magic_numbers(); // This is a very time consuming process, the magic numbers are already hardcoded
    test_bitboard();
}

BitBoard::~BitBoard(){
    std::cout << "BitBoard destructor" << std::endl;
}

void BitBoard::test_bitboard()
{   
    // parse_fen("8/3R4/8/1q1B4/8/8/8/8 w - -");
    // parse_fen(tricky_position);
    // parse_fen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    // parse_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    parse_fen(start_position);
    // parse_fen(killer_position);  

    int start = get_time_ms();

    perft_driver(6);
    
    int end = get_time_ms();

    std::cout << "Time taken to calculate legal_moves: " << end - start << " ms" << std::endl; 
    std::cout << "Total nodes: " << leaf_nodes << std::endl;
    std::cout << "Total captures: " << perf_captures << std::endl;
    std::cout << "Total en passant: " << perf_enpassant << std::endl;
    std::cout << "Total castles: " << perf_castlings << std::endl;
    std::cout << "Total promotions: " << perf_promotions << std::endl;
    std::cout << "Total checks: " << perf_checks << std::endl;
    // std::cout << "Nodes per second: " << leaf_nodes / ((end - start) / 1000) << std::endl;

    // print_move_list(move_list);
}


/*
<=========================================================================================>
<=========================================================================================>
<===============================BOARD MANIPULATION========================================>
<=========================================================================================>
<=========================================================================================>
*/



/*
<=========================================================================================>
<=========================================================================================>
<===============================MOVE GENERATION===========================================>
<=========================================================================================>
<=========================================================================================>
*/

inline void BitBoard::generate_moves(moves* move_list) // provides the pseudo-legals moves
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

inline int BitBoard::make_move(int move, int move_flag)
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
        if (get_move_capture(move))
        {
            make_move(move, all_moves);
        }
        else
        {
            return 0;
        }
    }

    return 0;
}

static inline void add_move(moves *move_list, int move) // Does not need to be a member function
{
    move_list->moves[move_list->count] = move;
    move_list->count++;
}


/*
<=========================================================================================>
<=========================================================================================>
<=====================================ATTACKS=============================================>
<=========================================================================================>
<=========================================================================================>
*/

inline int BitBoard::is_square_attacked(int square, int side) // Is the given side attacking the given square
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
    for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--) attacks |= (1ULL << (r * 8 + f));
    for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--) attacks |= (1ULL << (r * 8 + f));

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

inline U64  BitBoard::get_bishop_attacks(int square, U64 occupancy) // input occupancy is the board occupancy
{
    occupancy &= bishop_masks[square]; // See what squares are occupied in the bishop's vision
    occupancy *= bishop_magic_numbers[square]; // The next two steps are the process of retrieving the magic index
    occupancy >>= 64 - bishop_relevant_bits[square]; 

    return bishop_attacks[square][occupancy];
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

inline U64  BitBoard::get_rook_attacks(int square, U64 occupancy) // input occupancy is the board occupancy
{
    occupancy &= rook_masks[square]; // See what squares are occupied in the bishop's vision
    occupancy *= rook_magic_numbers[square]; // The next two steps are the process of retrieving the magic index
    occupancy >>= 64 - rook_relevant_bits[square]; 

    return rook_attacks[square][occupancy];
}

inline U64 BitBoard::get_queen_attacks(int square, U64 occupancy)
{
    return get_bishop_attacks(square, occupancy) | get_rook_attacks(square, occupancy);
}

void BitBoard::init_leapers_attacks()
{
    for (int square = 0; square < 64; square++)
    {
        pawn_attacks[white][square] = mask_pawn_attacks(white, square);
        pawn_attacks[black][square] = mask_pawn_attacks(black, square);
    
        knight_attacks[square] = mask_knight_attacks(square);

        king_attacks[square] = mask_king_attacks(square);       
    }
}

void BitBoard::init_sliders_attacks(int bishop)
{
    for (int square = 0; square < 64; square++)
    {
        U64 attack_mask;
        if (bishop)
        {
            bishop_masks[square] = mask_bishop_attacks(square);
            attack_mask = bishop_masks[square];
        }
        else
        {
            rook_masks[square] = mask_rook_attacks(square);
            attack_mask = rook_masks[square];
        }

        int relevant_bits_count = count_bits(attack_mask);

        int occupancy_indices = 1 << relevant_bits_count;

        for (int index = 0; index < occupancy_indices; index++)
        {
            U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);
            
            if (bishop)
            {
                int magic_index = occupancy * bishop_magic_numbers[square] >> (64 - bishop_relevant_bits[square]);
                bishop_attacks[square][magic_index] = bishop_attacks_on_the_fly(square, occupancy);
            }
            else
            {
                int magic_index = occupancy * rook_magic_numbers[square] >> (64 - rook_relevant_bits[square]);
                rook_attacks[square][magic_index] = rook_attacks_on_the_fly(square, occupancy);
            }
        }
    }

}

void BitBoard::print_attacked_square(int side)
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

                is_square_attacked(square, side) ? std::cout << "1 " : std::cout << "0 ";
            }
            std::cout << std::endl;
        }
        std::cout << "   a b c d e f g h\n" << std::endl;
}

/*
<=========================================================================================>
<=========================================================================================>
<==============================FEN MANIPULATION===========================================>
<=========================================================================================>
<=========================================================================================>
*/

void BitBoard::parse_fen(const char *fen)
{   
    // Reset the board
    memset(bitboards, 0ULL, sizeof(bitboards));
    memset(occupancies, 0ULL, sizeof(occupancies));
    side = 0;
    en_passant_square = no_sq;
    castle_rights = 0;
    halfmove = 0;
    fullmove = 0;

    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;

            if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z')) // If there's a piece
            {
                int piece = char_pieces[*fen];

                set_bit(bitboards[piece], square);

                fen++;
            }

            if (*fen >= '0' && *fen <= '9') // If there's an empty square
            {
                int offset = *fen - '0';

                file += offset;

                int piece = -1;
                for (int i = P; i <= k; i++)
                {
                    if (get_bit(bitboards[i], square))
                    {
                        piece = i;
                        break;
                    }
                }
                if (piece == -1) file--;

                fen++;
            }

            if (*fen == '/') fen++;
        }
    } // This double for loop will go through the fen characters relative to board position. In the end, the fen pointer is in the state variables.
    
    fen++;

    (*fen == 'w') ? side = 0 : side = 1; // Side to move

    fen += 2; // Skip the space and go to the castling rights

    while (*fen != ' ')
    {
        
        switch (*fen)
        {
        case 'K':
            castle_rights |= wk;
            break;
        case 'Q':
            castle_rights |= wq;
            break;
        case 'k':
            castle_rights |= bk;
            break; 
        case 'q':
            castle_rights |= bq;
            break;
        
        default:
            break;
        }

        fen++;
    }

    fen++;

    if (*fen != '-')
    {
        int file = fen[0] - 'a';
        int rank = 8 - (fen[1] - '0');

        en_passant_square = rank * 8 + file;
        fen += 3;
    }
    else 
    {
        en_passant_square = no_sq;
        fen += 2;
    }

    halfmove = atoi(fen);
    fen += 2;
    fullmove = atoi(fen);

    // Fill the occuppancies 

    for (int i = P; i <= K; i++)
    {
        occupancies[white] |= bitboards[i];
    }

    for (int i = p; i <= k; i++)
    {
        occupancies[black] |= bitboards[i];
    }

    occupancies[both] |= occupancies[white]; 
    occupancies[both] |= occupancies[black];
}

/*
<=========================================================================================>
<=========================================================================================>
<================================PRINT BOARDS=============================================>
<=========================================================================================>
<=========================================================================================>
*/

void BitBoard::print_board()
{
    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;
            if (!file)
            {
                std::cout << 8 - rank << "  ";
            }
            int piece = -1;
            for (int i = P; i <= k; i++)
            {
                if (get_bit(bitboards[i], square))
                {
                    piece = i;
                    break;
                }
            }
            std::cout << ((piece == -1) ? "." : unicode_pieces[piece]) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "   a b c d e f g h\n" << std::endl;
    std::cout << "   Side: " << (!side ? "White" : "Black") << std::endl;
    std::cout << "   En Passant: " << ((en_passant_square != no_sq) ? square_to_coords[en_passant_square] : "-") << std::endl;
    std::cout << "   Castling: " << ((castle_rights & wk) ? "K" : "") << 
                ((castle_rights & wq) ? "Q" : "") << ((castle_rights & bk) ? "k" : "") << 
                ((castle_rights & bq) ? "q" : "") << std::endl;
    std::cout << "   Half Move: " << halfmove << std::endl;
    std::cout << "   Full Move: " << fullmove << std::endl;
    std::cout << std::endl;
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

void BitBoard::print_move(int move)
{
    std::cout << square_to_coords[get_move_source(move)] << square_to_coords[get_move_target(move)] << promoted_pieces[get_move_promoted(move)] << std::endl;
}

void BitBoard::print_move_list(moves* move_list)
{   
    if (move_list->count == 0)
    {
        std::cout << "No moves available" << std::endl;
        return;
    }

    std::cout << "\n       move  piece  capture  double  enpassant  castling\n";
    
    for (int move_count = 0; move_count < move_list->count; move_count++)
    {
        int move = move_list->moves[move_count];
        
        std::string index = std::to_string(move_count + 1);
        
        std::cout << "  " << ((move_count + 1 < 10) ? " " + index : index) << ": " <<
                    square_to_coords[get_move_source(move)] << 
                    square_to_coords[get_move_target(move)] << 
                    promoted_pieces[get_move_promoted(move)] << "    " <<
                    unicode_pieces[get_move_piece(move)] <<  "       " <<
                    (bool)get_move_capture(move) <<  "       " <<
                    (bool)get_move_double(move) <<  "         " <<
                    (bool)get_move_enpassant(move) <<  "         " <<
                    (bool)get_move_castling(move) <<  "         " <<
                    std::endl;
        
    }
}

/*
<=========================================================================================>
<=========================================================================================>
<=====================================TIME================================================>
<=========================================================================================>
<=========================================================================================>
*/

int get_time_ms()
{
    struct timeval time_value;
    gettimeofday(&time_value, NULL);

    return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
}

/*
<=========================================================================================>
<=========================================================================================>
<=====================================TESTING=============================================>
<=========================================================================================>
<=========================================================================================>
*/

inline void BitBoard::perft_driver(int depth)
{
    if (depth == 0)
    {
        leaf_nodes++;
        return;
    }

    moves move_list[1]; 
    
    generate_moves(move_list);

    // print_move_list(move_list);

    for (int i = 0; i < move_list->count; i++)
    {
        copy_board();
    
        if (!make_move(move_list->moves[i], all_moves)) continue;

        perft_driver(depth - 1);
        
        if (get_move_capture(move_list->moves[i])) perf_captures++;
        if (get_move_enpassant(move_list->moves[i])) perf_enpassant++;
        if (get_move_castling(move_list->moves[i])) perf_castlings++;
        if (get_move_promoted(move_list->moves[i])) perf_promotions++;

        if (is_square_attacked(get_least_significant_bit((side == white) ? bitboards[K] : bitboards[k]), side ^1)) perf_checks++;

        restore_board();
    }   


}