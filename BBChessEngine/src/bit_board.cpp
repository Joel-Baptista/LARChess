#include <vector>
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
    init_leapers_attacks();
    init_sliders_attacks(bishop);
    init_sliders_attacks(rook);
    // init_magic_numbers(); // This is a very time consuming process, the magic numbers are already hardcoded
    // test_bitboard();

    parse_fen(start_position);
    // print_board();
}

BitBoard::~BitBoard(){
}

void BitBoard::test_bitboard()
{   
    // parse_fen("8/3R4/8/1q1B4/8/8/8/8 w - -");
    // parse_fen(tricky_position);
    parse_fen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    // parse_fen("r3k2r/pPppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    // parse_fen(start_position);
    // parse_fen(killer_position);  

    // std::vector<std::string> starting_states = {
    //     "rnbq1rk1/pppp1ppp/4pn2/8/1bPP4/2N1P3/PP3PPP/R1BQKBNR w KQ - 1 5", // Nimzo-Indian
    //     "r1bqkbnr/pp1ppppp/2n5/2p5/3PP3/5N2/PPP2PPP/RNBQKB1R b KQkq d3 0 3", // Sicilian
    //     "rnbqkbnr/pp2pppp/2p5/3p4/3PP3/8/PPP2PPP/RNBQKBNR w KQkq d6 0 3", // Caro-Kann
    //     "rnbqkb1r/ppp2ppp/4pn2/3p4/3PP3/2N5/PPP2PPP/R1BQKBNR w KQkq - 2 4", // French Defense
    //     "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3", // Italian Game
    //     "r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3", // Ruy Lopez
    //     "rnbqkb1r/pppp1ppp/5n2/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3", // Pretov's Defense
    //     "rn1qkb1r/pp2pppp/2p2n2/3p1b2/2PP4/4PN2/PP3PPP/RNBQKB1R w KQkq - 3 5", // Queen's Gambit
    //     "rnbqkbnr/ppp1pppp/8/3p4/3P1B2/8/PPP1PPPP/RN1QKBNR b KQkq - 1 2", // London System
    //     "rnbqkb1r/ppppp1pp/5n2/6B1/3Pp3/2N5/PPP2PPP/R2QKBNR b KQkq - 3 4", // Dutch Defense
    // };

    
    // bool game_end = false;
    // int vanilla_alpha = 0;
    // int offset = 0;

    // int vanilla_alpha_wins = 0;
    // int quien_alpha_wins = 0;

    // int depth = 4;

    // for (int i=0; i < starting_states.size() * 2; i++)
    // {
    //     std::cout << "Game " << i + 1 << std::endl;
    //     if (vanilla_alpha == 0 && i >= starting_states.size())
    //     {
    //         offset = starting_states.size();
    //         vanilla_alpha = 1;
    //     }

    //     parse_fen(starting_states[i - offset].c_str());
    //     game_end = false;

    //     while (!game_end)
    //     {

    //         // print_board();
    //         bot_best_move = 0;
    //         if (side == vanilla_alpha)
    //         {
    //             alpha_beta(depth, -1000000, 1000000);
    //         }
    //         else
    //         {
    //             alpha_beta(depth, -1000000, 1000000, true);
    //         }
    //         // print_move(bot_best_move);
    //         if (bot_best_move == 0)
    //         {
    //             game_end = true;
    //             std::cout << "Game Over" << std::endl;

    //             if (is_square_attacked(get_least_significant_bit((side == white) ? bitboards[K] : bitboards[k]) , side^1))
    //             {
    //                 std::cout << ((side == white) ? "Black" : "White") << " Wins!" << std::endl;

    //                 if ((side == white && vanilla_alpha == 1) || (side == black && vanilla_alpha == 0))
    //                 {
    //                     vanilla_alpha_wins++;
    //                 }
    //                 else
    //                 {
    //                     quien_alpha_wins++;
    //                 }

    //                 break;
    //             }
    //             else
    //             {
    //                 std::cout << "Stalemate" << std::endl;
    //             }


    //             break;
    //         }
    //         else
    //         {
    //             make_move(bot_best_move, all_moves);

    //             if (halfmove > 100)
    //             {
    //                 game_end = true;
    //                 std::cout << "Game Over" << std::endl;
    //                 std::cout << "Stalemate" << std::endl;
    //                 break;
    //             }
            
    //         }

    //     }
    // }

    
    // std::cout << "Vanilla Alpha: " << vanilla_alpha_wins << std::endl;
    // std::cout << "Quien Alpha: " << quien_alpha_wins << std::endl;
    // std::cout << "Draws: " << (2 * starting_states.size()) - vanilla_alpha_wins - quien_alpha_wins << std::endl;
    // std::cout << "Total Games: " << (2 * starting_states.size()) << std::endl;


    // int input_move = parse_move("c3b5");

    // std::cout << move_to_uci(input_move) << std::endl;

    // print_board();
    // if (input_move)
    // {
    //     make_move(input_move, all_moves);
    //     print_board();
    // }
    // else
    // {
    //     std::cout << "Invalid move" << std::endl;
    // }

    // bitboard_to_board();

    // int start = get_time_ms();

    // perft_test(4);
    
    // int end = get_time_ms();

    moves move_list_original;
    moves move_list;
    moves move_list_alpha;

    generate_moves(&move_list_original);
    generate_alpha_moves(&move_list, &move_list_alpha);


    std::cout << "Legal Moves Original: " << move_list_original.count << std::endl;
    std::cout << "Legal Moves: " << move_list.count << std::endl;
    std::cout << "Legal Moves Alpha: " << move_list_alpha.count << std::endl;

    moves mov;
    get_alpha_moves(&mov);

    std::cout << "Full Legal Moves Alpha: " << mov.count << std::endl;


}


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

int BitBoard::make_player_move(const char *move)
{
    int input_move = parse_move(move);
    if (input_move)
    {

        copy_board();

        if (make_move(input_move, all_moves))
        {
            return 1;
        }
        else
        {
            restore_board();
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

int BitBoard::make_bot_move(int move)
{
    if (move)
    {
        copy_board();
        if (make_move(move, all_moves))
        {
            return 1;
        }
        else
        {
            restore_board();
            return 0;
        }
    }
    else
    {
        return 0;
    }
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
<==============================FEN/UCI MANIPULATION===========================================>
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

int BitBoard::parse_move(const char* move_string)
{
    moves move_list[1];
    generate_moves(move_list);

    int source_square = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;
    int target_square = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;


    int promoted_piece = 0;
    
    for (int i = 0; i < move_list->count; i++)
    {
        if (get_move_source(move_list->moves[i]) == source_square && get_move_target(move_list->moves[i]) == target_square)
        {
            int promoted = get_move_promoted(move_list->moves[i]);
            if (promoted)
            {
                if      ((promoted == Q || promoted == q) && move_string[4] == 'q') return move_list->moves[i];
                else if ((promoted == R || promoted == r) && move_string[4] == 'r') return move_list->moves[i];
                else if ((promoted == N || promoted == n) && move_string[4] == 'n') return move_list->moves[i];
                else if ((promoted == B || promoted == b) && move_string[4] == 'b') return move_list->moves[i];
                continue;
            }

            return move_list->moves[i];
        }
    }

    return 0;
}

std::string BitBoard::move_to_uci(int move)
{
    std::string move_string = "";

    move_string += square_to_coords[get_move_source(move)];
    move_string += square_to_coords[get_move_target(move)];

    int promoted = get_move_promoted(move);

    if (promoted)
    {
        switch (promoted)
        {
        case Q:
            move_string += "q";
            break;
        case R:
            move_string += "r";
            break;
        case N:
            move_string += "n";
            break;
        case B:
            move_string += "b";
            break;
        
        default:
            break;
        }
    }

    return move_string;
}

std::array<std::array<int, 8>, 8> BitBoard::bitboard_to_board()
{
    std::array<std::array<int, 8>, 8> board_array;
    
    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;
            
            int piece = -1;
            for (int i = P; i <= k; i++)
            {
                if (get_bit(bitboards[i], square))
                {
                    piece = i;
                    break;
                }
            }

            if (piece == -1)
            {
                board_array[rank][file] = 0;
            }
            else
            {
                board_array[rank][file] = board_conversion[piece];
            }

        }
    }

    return board_array;
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

void BitBoard::perft_test(int depth)
{
    std::cout << "\n    Performance Test\n\n" << std::endl;

    int start = get_time_ms();
    moves move_list[1]; 
    
    generate_moves(move_list);

    // print_move_list(move_list);

    for (int i = 0; i < move_list->count; i++)
    {
        copy_board();
    
        if (!make_move(move_list->moves[i], all_moves)) continue;

        long long cummulative_nodes = leaf_nodes;

        perft_driver(depth - 1);

        long long old_leaf_nodes = leaf_nodes - cummulative_nodes;
        
        if (get_move_capture(move_list->moves[i])) perf_captures++;
        if (get_move_enpassant(move_list->moves[i])) perf_enpassant++;
        if (get_move_castling(move_list->moves[i])) perf_castlings++;
        if (get_move_promoted(move_list->moves[i])) perf_promotions++;

        if (is_square_attacked(get_least_significant_bit((side == white) ? bitboards[K] : bitboards[k]), side ^1)) perf_checks++;

        restore_board();

        std::cout << "   Move: ";
        std::cout << square_to_coords[get_move_source(move_list->moves[i])] << 
                     square_to_coords[get_move_target(move_list->moves[i])] << 
                     promoted_pieces[get_move_promoted(move_list->moves[i])] << 
                     "  Nodes: "  <<  old_leaf_nodes  <<  std::endl;
    }

    int end = get_time_ms();

    std::cout << "\n\n     Depth: " << depth << "\n";
    std::cout << "     Nodes: " << leaf_nodes << "\n";
    std::cout << "      Time: " << end - start << " ms\n";
    std::cout << "  Captures: " << perf_captures << std::endl;
    std::cout << "En passant: " << perf_enpassant << std::endl;
    std::cout << "   Castles: " << perf_castlings << std::endl;
    std::cout << "Promotions: " << perf_promotions << std::endl;
    std::cout << "    Checks: " << perf_checks << std::endl;

}

/*
<=========================================================================================>
<=========================================================================================>
<=======================================BOTS==============================================>
<=========================================================================================>
<=========================================================================================>
*/

float BitBoard::board_evaluation()
{
    float evaluation = 0;

    if (halfmove > 100) return 0;

    for (int rank=0; rank < 8; rank++)
    {
        for (int file=0; file < 8; file++)
        {
            int square = rank * 8 + file;
            int piece = -1;
            for (int i = P; i <= k; i++)
            {
                if (get_bit(bitboards[i], square))
                {
                    piece = i;
                    break;
                }
                
            }

            if (piece == -1) continue;

            // if (piece == K || piece == k) 
            // {
            //     evaluation += piece_values[piece] * king_safety_value[square];
            //     continue;
            // }

            if (piece == N || piece == n) 
            {   
                int piece_player = (piece == N) ? 0 : 1;
                int is_protected = is_square_attacked(square, piece_player);

                U64 attacks = knight_attacks[square] & ((piece_player == white) ? ~occupancies[white] : ~occupancies[black]);
                // evaluation += piece_values[piece];
                evaluation += piece_values[piece] * (0.5 + (count_bits(attacks)/8.0)) * (is_protected ? 1.1 : 0.9);
                
                continue;
            }
            
            if (piece == B || piece == b) 
            {
                int piece_player = (piece == B) ? 0 : 1;
                int is_protected = is_square_attacked(square, piece_player);

                U64 attacks = get_bishop_attacks(square, occupancies[both]) & ((piece_player == white) ? ~occupancies[white] : ~occupancies[black]);
                // evaluation += piece_values[piece];
                evaluation += piece_values[piece] * (0.5 + (count_bits(attacks)/13.0)) * (is_protected ? 1.1 : 0.9);
                continue;
            }

            if (piece == R || piece == r) 
            {
                int piece_player = (piece == R) ? 0 : 1;
                int is_protected = is_square_attacked(square, piece_player);

                U64 attacks = get_rook_attacks(square, occupancies[both]) & ((piece_player == white) ? ~occupancies[white] : ~occupancies[black]);
                // evaluation += piece_values[piece];
                evaluation += piece_values[piece] * (0.5 + (count_bits(attacks)/14.0)) * (is_protected ? 1.1 : 0.9);
                continue;
            }

            if (piece == Q || piece == q) 
            {
                int piece_player = (piece == Q) ? white : black;
                int is_protected = is_square_attacked(square, piece_player);

                U64 attacks = get_queen_attacks(square, occupancies[both]) & ((piece_player == white) ? ~occupancies[white] : ~occupancies[black]);
                // evaluation += piece_values[piece];
                evaluation += piece_values[piece] * (0.5 + (count_bits(attacks)/27.0)) * (is_protected ? 1.1 : 0.9);
                continue;
            }

            if (piece == P || piece == p) 
            {
                int piece_player = (piece == P) ? 0 : 1;
                int is_protected = is_square_attacked(square, piece_player);

                // evaluation += piece_values[piece];
                evaluation += piece_values[piece] * piece_positional_value[square] * (is_protected ? 1.1 : 0.9);
                continue;
            }
        }   
    }

    return evaluation;
}

inline float BitBoard::quiescence(float alpha, float beta)
{

    float eval = board_evaluation();
    float best_eval = (side == white) ? -1000000.0 : 1000000.0;
    
    // std::cout << "Quiescene Eval: " << eval << " Alpha: " << alpha << " Beta: " << beta << std::endl;

    if (side == white)
    {
        if (eval > best_eval)
        {
            best_eval = eval;
        }
        alpha = std::max(alpha, eval);

        if (alpha >= beta)
        {
            return best_eval;
        }
    }
    else
    {
        if (eval < best_eval)
        {
            best_eval = eval;
        }

        beta = std::min(beta, eval);

        if (alpha >= beta)
        {
            return best_eval;
        }
    }   


    leaf_nodes++;

    int legal_movel_cntr = 0;
    int capture_counter = 0;
    int is_king_in_check = is_square_attacked((side == white) ? get_least_significant_bit(bitboards[K]) : get_least_significant_bit(bitboards[k]), side ^ 1);

    moves move_list[1];

    generate_moves(move_list);

    int caputure_moves = 0;
    for (int i = 0; i < move_list->count; i++)
    {
        if (get_move_capture(move_list->moves[i]))
        {
            caputure_moves++;
        }
    }


    // print_board();
    if (side == white)
    {
        for (int i = 0; i < move_list->count; i++)
        {
            copy_board();

            ply++;

            if (!make_move(move_list->moves[i], only_captures)) 
            {
                ply--;
                continue;
            }   
            capture_counter++;
            legal_movel_cntr++;

            float eval = quiescence(alpha, beta);
            ply--;
            if (eval > best_eval)
            {
                best_eval = eval;
            }
            alpha = std::max(alpha, eval);

            if (alpha >= beta)
            {
                restore_board();
                break;
            }

            restore_board();
        }
    }
    else
    {

    for (int i = 0; i < move_list->count; i++)
        {
            copy_board();

            ply++;

            if (!make_move(move_list->moves[i], only_captures))
            {
                ply--;
                continue;
            }
            capture_counter++;
            legal_movel_cntr++;

            float eval = quiescence(alpha, beta);
            ply--;
            if (eval < best_eval)
            {
                best_eval = eval;
            }

            beta = std::min(beta, eval);

            if (alpha >= beta)
            {
                restore_board();
                break;
            }

            restore_board();
        }

    }

    // std::cout << "Capture Counter: " << capture_counter << std::endl;
    // std::cout << "Capture Moves: " << caputure_moves << std::endl;

    return best_eval;

}


float BitBoard::alpha_beta(int depth, float alpha, float beta, bool quien)

{
            
    // std::cout << "Depth: " << depth << std::endl;
    if (depth == 0) 
    {
        leaf_nodes++;   
        if (quien)
            return quiescence(alpha, beta);
   
        return board_evaluation();
    }


    int legal_movel_cntr = 0;
    int is_king_in_check = is_square_attacked((side == white) ? get_least_significant_bit(bitboards[K]) : get_least_significant_bit(bitboards[k]), side ^ 1);

    moves move_list[1];

    generate_moves(move_list);
    // print_board();
    float best_eval = (side == white) ? -1000000.0f : 1000000.0f;

    if (side == white)
    {
        for (int i = 0; i < move_list->count; i++)
        {
            copy_board();

            ply++;

            if (!make_move(move_list->moves[i], all_moves)) 
            {
                ply--;
                continue;
            }

            legal_movel_cntr++;

            float eval = alpha_beta(depth - 1, alpha, beta, quien);
            ply--;
            if (eval > best_eval)
            {
                best_eval = eval;

                if (ply == 0)
                    bot_best_move = move_list->moves[i];
            }
            alpha = std::max(alpha, eval);

            if (alpha >= beta)
            {
                restore_board();
                break;
            }

            restore_board();
        }
    }
    else
    {

    for (int i = 0; i < move_list->count; i++)
        {
            copy_board();

            ply++;
        
            if (!make_move(move_list->moves[i], all_moves)) 
            {
                ply--;
                continue;
            }

            legal_movel_cntr++;

            float eval = alpha_beta(depth - 1, alpha, beta, quien);
            ply--;
            if (eval < best_eval)
            {
                best_eval = eval;

                if (ply == 0)
                    bot_best_move = move_list->moves[i];
            }

            beta = std::min(beta, eval);

            if (alpha >= beta)
            {
                restore_board();
                break;
            }

            restore_board();
        }

    }

    if (legal_movel_cntr == 0)
    {
        if (is_king_in_check)
        {
            return (side == white) ? -49000 - ply : 49000 + ply;
        }
        else
        {
            return 0;
        }
    }
    return best_eval;
}

void BitBoard::get_alpha_moves(moves* move_list)
{

    moves move_list_bb;
    moves move_list_alpha;

    generate_alpha_moves(&move_list_bb, &move_list_alpha);

    if (move_list_alpha.count != move_list_bb.count)
    {
        std::cout << "Error in move generation" << std::endl;
        return;
    }

    for (int i = 0; i < move_list_alpha.count; i++)
    {
        copy_board();

        if (!make_move(move_list_bb.moves[i], all_moves)) 
        {
            restore_board();
            continue;
        }
        add_move(move_list, move_list_alpha.moves[i]);
        
        restore_board();
    }

}

inline void BitBoard::generate_alpha_moves(moves* move_list, moves* move_list_alpha)
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