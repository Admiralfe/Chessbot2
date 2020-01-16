#include <assert.h>

#include "attacks.h"
#include "bitboard.h"
#include "position.h"
#include "movegen.h"
#include "tests.h"
#include "types.h"

void run_all_tests() {
    test_FEN();
    test_bitshifts();
    test_attack_sets();
    test_ray_attacks();
    test_attacks_from();
}

void test_FEN() {
    //Test the starting position
    struct Position pos = pos_from_FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    struct Position pos_ep = pos_from_FEN("rnbqkbnr/ppp2ppp/8/3Pp3/8/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 1");

    //Check some of the pieces
    assert(pos.piece_list[3] == WHITE_QUEEN && pos.piece_list[63] == BLACK_ROOK);

    //Check the bitboards;
    assert(pos.pawns[WHITE] == Rank2BB);
    assert(pos.pawns[BLACK] == Rank7BB);

    assert(pos.knights[WHITE] == (set_bit(b1) | set_bit(g1)));
    assert(pos.bishops[BLACK] == (set_bit(c8) | set_bit(f8)));

    assert(pos.occupied_squares[WHITE] == (Rank1BB | Rank2BB));
    assert(pos.occupied_squares[BLACK] == (Rank7BB | Rank8BB));
    
    //Castling rights
    assert(pos.can_kingside_castle[WHITE] && pos.can_queenside_caslte[WHITE]
        && pos.can_kingside_castle[BLACK] && pos.can_queenside_caslte[BLACK]);
    
    //Should be white's
    assert(pos.side_to_move == WHITE);

    //Should be no ep square.
    assert(pos.ep_square == SQUARE_EMPTY);
    assert(pos_ep.ep_square == e6);

    //50 move clock should be 0
    assert(pos.half_move_clock == 0);
}

void test_bitshifts() {
    u64 test_bb;
    test_bb = set_bit(e4);
    assert(shift_N(test_bb) == set_bit(e5) && shift_NE(test_bb) == set_bit(f5)
            && shift_E(test_bb) == set_bit(f4) && shift_SE(test_bb) == set_bit(f3)
            && shift_S(test_bb) == set_bit(e3) && shift_SW(test_bb) == set_bit(d3)
            && shift_W(test_bb) == set_bit(d4) && shift_NW(test_bb) == set_bit(d5));
        

    test_bb = set_bit(a1);
    assert(shift_W(test_bb) == 0ULL && shift_S(test_bb) == 0ULL);

    test_bb = set_bit(h8);
    assert(shift_E(test_bb) == 0ULL && shift_N(test_bb) == 0ULL);
}

void test_attack_sets() {
    fill_attack_sets();

    //Check pawn attacks
    assert(attack_set.pawn[WHITE][e4] == (set_bit(d5) | set_bit(f5))
        && attack_set.pawn[BLACK][e4] == (set_bit(d3) | set_bit(f3))
        && attack_set.pawn[WHITE][a2] == (set_bit(b3))
        && attack_set.pawn[BLACK][h6] == set_bit(g5));

    //Check knight attacks
    assert(attack_set.knight[d4] == 
        ( set_bit(e6) | set_bit(f5) | set_bit(f3)
        | set_bit(e2) | set_bit(c2) | set_bit(b3)
        | set_bit(b5) | set_bit(c6)));
    
    assert(attack_set.knight[h1] == (set_bit(g3) | set_bit(f2)));
    assert(attack_set.knight[a7] == (set_bit(c8) | set_bit(c6) | set_bit(b5)));

    assert(attack_set.king[e4] == (set_bit(e5) | set_bit(f5) | set_bit(f4)
                                    | set_bit(f3) | set_bit(e3) | set_bit(d3)
                                    | set_bit(d4) | set_bit(d5)));
}

void test_ray_attacks() {
    fill_attack_rays();

    assert(attack_rays[e4][NORTHEAST] == (set_bit(f5) | set_bit(g6) | set_bit(h7)));
    assert(attack_rays[e4][NORTH] == (set_bit(e5) | set_bit(e6) | set_bit(e7) | set_bit(e8)));
    assert(attack_rays[e4][EAST] == (set_bit(f4) | set_bit(g4) | set_bit(h4)));
    assert(attack_rays[e4][SOUTHEAST] == (set_bit(f3) | set_bit(g2) | set_bit(h1)));
    assert(attack_rays[e4][SOUTH] == (set_bit(e3) | set_bit(e2) | set_bit(e1)));
    assert(attack_rays[e4][SOUTHWEST] == (set_bit(d3) | set_bit(c2) | set_bit(b1)));
    assert(attack_rays[e4][WEST] == (set_bit(d4) | set_bit(c4) | set_bit(b4) | set_bit(a4)));
    assert(attack_rays[e4][NORTHWEST] == (set_bit(d5) | set_bit(c6) | set_bit(b7) | set_bit(a8)));

    assert(attack_rays[h8][NORTH] == 0ULL);
    assert(attack_rays[a1][WEST] == 0ULL);
    assert(attack_rays[b2][NORTHWEST] == set_bit(a3));
    assert(attack_rays[g7][SOUTHEAST] == set_bit(h6));
}

void test_attacks_from() {
    fill_attack_rays();

    struct Position pos = pos_from_FEN("8/4rn2/4k3/2B5/3P1Q2/4K3/8/5R1b w - - 0 1");

    u64 attacks_queen = attacks_from(QUEEN, &pos, f4);
    u64 attacks_bishop = attacks_from(BISHOP, &pos, c5);
    u64 attacks_rook = attacks_from(ROOK, &pos, f1);

    print_position(pos);

    printf("Queen attacks\n");
    print_bitboard(attacks_queen);

    printf("Bishop attacks\n");
    print_bitboard(attacks_bishop);

    printf("Rook attacks\n");
    print_bitboard(attacks_rook);
}

void test_movegen_pawns() {
    fill_attack_sets();

    struct Position pos = pos_from_FEN("rnbqkbnr/ppp2ppp/8/3Pp3/8/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 1");
    struct Position pos2 = pos_from_FEN("r1bqkbnr/1ppp1ppp/p1B5/4p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 0 1");
    struct Position pos_promotions = pos_from_FEN("1bn5/PP6/8/3k1K2/8/8/8/8 w - - 0 1");

    struct Move move_list[256];
    struct Move move_list2[256];
    struct Move move_list_prom[256];
 
    int num_moves = generate_pawn_moves(move_list, pos);
    int num_moves2 = generate_pawn_moves(move_list2, pos2);
    int num_moves_prom = generate_pawn_moves(move_list_prom, pos_promotions);

    printf("Moves in position 1:\n");
    for (int i = 0; i < num_moves; ++i)
        print_move(move_list[i], pos);

    printf("\nMoves in position 2:\n");
    for (int i = 0; i < num_moves2; ++i)
        print_move(move_list2[i], pos2);

    printf("\nMoves in position 3:\n");
    for (int i = 0; i < num_moves_prom; ++i)
        print_move(move_list_prom[i], pos_promotions);
}