#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "attacks.h"
#include "bitboard.h"
#include "position.h"
#include "movegen.h"
#include "stack.h"
#include "tables.h"
#include "tests.h"
#include "types.h"

void run_all_tests() {
    printf("Starting tests...\n");
    init_LUTs();
    printf("LUT initialization passed\n");
    test_FEN();
    printf("FEN passed\n");
    test_bitshifts();
    printf("Bitshift test passed\n");
    test_attack_sets();
    printf("Attack sets test passed\n");
    test_ray_attacks();
    printf("Ray attacks test passed\n");
    test_attacks_from();
    printf("Attacks_from test passed\n");
    test_movegen();
    printf("Move generation test passed\n");
    test_in_between_LUT();
    printf("in_between_LUT test passed\n");
    test_attackers_to();
    printf("attackers_to test passed\n");
    test_make_move();
    printf("make / unmake move test passed\n");
    test_stack();
    printf("Stack test passed\n");
    test_legal_move_check();
    printf("Legal move check test passed\n");
    printf("All tests passed!\n");
}

void test_FEN() {
    //Test the starting position
    struct Position pos = pos_from_FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    struct Position pos_ep = pos_from_FEN("rnbqkbnr/ppp2ppp/8/3Pp3/8/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 1");

    //Check some of the pieces
    assert(pos.piece_list[3] == WHITE_QUEEN && pos.piece_list[63] == BLACK_ROOK);

    //Check the bitboards;
    assert(pos.piece_bb[PAWN][WHITE] == Rank2BB);
    assert(pos.piece_bb[PAWN][BLACK] == Rank7BB);

    assert(pos.piece_bb[KNIGHT][WHITE] == (set_bit(b1) | set_bit(g1)));
    assert(pos.piece_bb[BISHOP][BLACK] == (set_bit(c8) | set_bit(f8)));

    assert(pos.occupied_squares[WHITE] == (Rank1BB | Rank2BB));
    assert(pos.occupied_squares[BLACK] == (Rank7BB | Rank8BB));

    //Castling rights
    assert(pos.can_kingside_castle[WHITE] && pos.can_queenside_castle[WHITE]
        && pos.can_kingside_castle[BLACK] && pos.can_queenside_castle[BLACK]);

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

    assert((*attack_rays_ptr)[e4][NORTHEAST] == (set_bit(f5) | set_bit(g6) | set_bit(h7)));
    assert((*attack_rays_ptr)[e4][NORTH] == (set_bit(e5) | set_bit(e6) | set_bit(e7) | set_bit(e8)));
    assert((*attack_rays_ptr)[e4][EAST] == (set_bit(f4) | set_bit(g4) | set_bit(h4)));
    assert((*attack_rays_ptr)[e4][SOUTHEAST] == (set_bit(f3) | set_bit(g2) | set_bit(h1)));
    assert((*attack_rays_ptr)[e4][SOUTH] == (set_bit(e3) | set_bit(e2) | set_bit(e1)));
    assert((*attack_rays_ptr)[e4][SOUTHWEST] == (set_bit(d3) | set_bit(c2) | set_bit(b1)));
    assert((*attack_rays_ptr)[e4][WEST] == (set_bit(d4) | set_bit(c4) | set_bit(b4) | set_bit(a4)));
    assert((*attack_rays_ptr)[e4][NORTHWEST] == (set_bit(d5) | set_bit(c6) | set_bit(b7) | set_bit(a8)));

    assert((*attack_rays_ptr)[h8][NORTH] == 0ULL);
    assert((*attack_rays_ptr)[a1][WEST] == 0ULL);
    assert((*attack_rays_ptr)[b2][NORTHWEST] == set_bit(a3));
    assert((*attack_rays_ptr)[g7][SOUTHEAST] == set_bit(h6));
}

void test_attacks_from() {
    fill_attack_rays();

    struct Position pos = pos_from_FEN("8/4rn2/4k3/2B5/3P1Q2/4K3/8/5R1b w - - 0 1");

    u64 attacks_queen = attacks_from(QUEEN, &pos, f4);
    u64 attacks_bishop = attacks_from(BISHOP, &pos, c5);
    u64 attacks_rook = attacks_from(ROOK, &pos, f1);

    print_position(&pos);

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

    int num_moves = generate_pawn_moves(move_list, &pos);
    int num_moves2 = generate_pawn_moves(move_list2, &pos2);
    int num_moves_prom = generate_pawn_moves(move_list_prom, &pos_promotions);

    printf("Moves in position 1:\n");
    for (int i = 0; i < num_moves; ++i)
        print_move(move_list[i], &pos);

    printf("\nMoves in position 2:\n");
    for (int i = 0; i < num_moves2; ++i)
        print_move(move_list2[i], &pos2);

    printf("\nMoves in position 3:\n");
    for (int i = 0; i < num_moves_prom; ++i)
        print_move(move_list_prom[i], &pos_promotions);
}

void test_castling_rights() {
    struct Position pos1 = pos_from_FEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

    assert(can_kingside_castle(WHITE, &pos1)
        && can_queenside_castle(WHITE, &pos1)
        && can_kingside_castle(BLACK, &pos1)
        && can_queenside_castle(BLACK, &pos1));

    struct Position pos2 = pos_from_FEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w - - 0 1");

    //Castling shouldn't be legal here because of castling rights not being set in FEN string.
    assert(!can_kingside_castle(WHITE, &pos2)
        && !can_queenside_castle(WHITE, &pos2)
        && !can_kingside_castle(BLACK, &pos2)
        && !can_queenside_castle(BLACK, &pos2));

    struct Position pos3 = pos_from_FEN("r3kb1r/p1ppqp2/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPB1PPP/R2BK2R w KQkq - 0 1");

    //Some of the castlings should be blocked in this position.
    assert(can_kingside_castle(WHITE, &pos3)
        && !can_queenside_castle(WHITE, &pos3)
        && !can_kingside_castle(BLACK, &pos3)
        && can_queenside_castle(BLACK, &pos3));
}

void test_movegen() {
    fill_attack_rays();
    fill_attack_sets();

    struct Position pos_starting = pos_from_FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    struct Position pos_captures = pos_from_FEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    struct Position pos_checkmate = pos_from_FEN("rnb1kbnr/pppp1ppp/4p3/8/5PPq/8/PPPPP2P/RNBQKBNR w KQkq - 0 1");
    printf("%d\n", pos_captures.piece_list[c1]);

    struct Move move_list_starting[256];
    struct Move move_list_captures[256];
    struct Move move_list_checkmate[256];

    int num_moves = generate_moves(move_list_starting, &pos_starting);
    int num_moves_captures = generate_moves(move_list_captures, &pos_captures);
    int num_moves_checkmate = generate_moves(move_list_checkmate, &pos_checkmate);
    for (int i = 0; i < num_moves_checkmate; ++i)
        print_move(move_list_checkmate[i], &pos_checkmate);
    assert(num_moves_checkmate == 0);

    printf("First position:\n");
    print_position(&pos_starting);
    printf("Legal moves in position 1:\n");
    for (int i = 0; i < num_moves; ++i)
        print_move(move_list_starting[i], &pos_starting);

    printf("Second position:\n");
    print_position(&pos_captures);
    printf("\nLegal moves in position 2\n");
    for (int i = 0; i < num_moves_captures; ++i)
        print_move(move_list_captures[i], &pos_captures);
}

void test_in_between_LUT() {
    fill_inbetween_LUT();

    //Check symmetry
    assert((*in_between_LUT_ptr)[e4][e1] == (*in_between_LUT_ptr)[e1][e4]);
    assert((*in_between_LUT_ptr)[a1][g8] == (*in_between_LUT_ptr)[g8][a1]);

    assert((*in_between_LUT_ptr)[c2][c5] == (set_bit(c3) | set_bit(c4)));
    assert((*in_between_LUT_ptr)[a1][a8] == (set_bit(a2) | set_bit(a3) | set_bit(a4) | set_bit(a5) | set_bit(a6) | set_bit(a7)));
    assert((*in_between_LUT_ptr)[e3][b3] == (set_bit(d3) | set_bit(c3)));
    assert((*in_between_LUT_ptr)[g8][c4] == (set_bit(f7) | set_bit(e6) | set_bit(d5)));
    assert((*in_between_LUT_ptr)[b5][e2] == (set_bit(c4) | set_bit(d3)));

    assert((*in_between_LUT_ptr)[c3][g4] == 0ULL);
    assert((*in_between_LUT_ptr)[f2][d7] == 0ULL);
    assert((*in_between_LUT_ptr)[d4][d4] == 0ULL);
    assert((*in_between_LUT_ptr)[a1][a2] == 0ULL);
}

void test_attackers_to() {
    init_LUTs();
    struct Position test_pos_1 = pos_from_FEN("1r1k2nr/3b1n2/3pp3/R4PR1/1N4N1/8/4P1B1/3QK3 b KQkq e6 0 1");
    struct Position test_pos_2 = pos_from_FEN("4k1RQ/3PPP2/6B1/8/8/8/8/8 b - - 0 1");
    struct Position test_pos_3 = pos_from_FEN("5k2/8/8/8/2q2r2/7b/3n2p1/1q3K2 w - - 0 1");
    enum Side side1 = test_pos_1.side_to_move;
    enum Side side2 = test_pos_2.side_to_move;
    enum Side side3 = test_pos_3.side_to_move;
    u64 attacks_1 = attackers_to(d5, &test_pos_1, side1);
    print_bitboard(attacks_1);
    assert(attacks_1 == (set_bit(a5) | set_bit(b4) | set_bit(d1) | set_bit(g2)));

    u64 attacks_2 = attackers_to(e8, &test_pos_2, side2);
    print_bitboard(attacks_2);
    assert(attacks_2 == (set_bit(d7) | set_bit(f7) | set_bit(g8)));

    u64 attacks_3 = attackers_to(f1, &test_pos_3, side3);
    print_bitboard(attacks_3);
    assert(attacks_3 == (set_bit(b1) | set_bit(c4) | set_bit(d2) | set_bit(g2) | set_bit(f4)));
}

static bool bitboard_piece_list_consistent(const struct Position *pos) {
    for (int sq = 0; sq < 64; ++sq) {
        if (pos->piece_list[sq] == PIECE_EMPTY) {
            //Check that the occupancy bb does not have the bit set
            if (is_set(sq, pos->occupied_squares[WHITE]))
                return false;
            //Check that all the piece bitboards are empty there
            for (enum Piece_type pt = PAWN; pt <= KING; ++pt) {
                if (is_set(sq, pos->piece_bb[pt][WHITE]) || is_set(sq, pos->piece_bb[pt][BLACK]))
                    return false;
            }
        }

        else {
            enum Piece piece = pos->piece_list[sq];
            enum Piece_type piece_type = to_piece_type(piece);
            enum Side color = piece_color(piece);
            enum Side other_side_color = abs(color - 1);

            //Check that the occupancy bitboard is set for the right side.
            if (!is_set(sq, pos->occupied_squares[color]) || is_set(sq, pos->occupied_squares[other_side_color]))
                return false;

            //Check that the bitboard for the piece is set
            if (!is_set(sq, pos->piece_bb[piece_type][color]))
                return false;

            //All other bitboards should have that bit unset.
            if (is_set(sq, pos->piece_bb[piece_type][other_side_color]))
                return false;

            for (enum Piece_type pt_idx = PAWN; pt_idx <= KING; ++pt_idx) {
                if (pt_idx != piece_type)
                    if (is_set(sq, pos->piece_bb[pt_idx][WHITE]) || is_set(sq, pos->piece_bb[pt_idx][BLACK]))
                        return false;
            }
        }
    }

    return true;
}

void test_make_move() {
    MS_Stack *move_state_stk = stk_create(INIT_STACK_SIZE);
    struct Position test_pos = pos_from_FEN("4k3/7p/8/3Pp3/5r2/8/3BQPN1/R3K2R w KQkq - 0 1");
    struct Move m1 = create_regular_move(g2, f4);
    struct Move m2 = create_special_move(CASTLING, PT_NULL, e1, g1); //Kingside castling
    struct Move m3 = create_special_move(CASTLING, PT_NULL, e1, c1); //Queenside castling
    struct Move m4 = create_special_move(ENPASSANT, PT_NULL, d5, e6); //EP capture
    struct Move m5 = create_regular_move(h7, h6);

    assert(bitboard_piece_list_consistent(&test_pos));
    assert(test_pos.side_to_move == WHITE);

    make_move(m1, &test_pos, move_state_stk);
    assert(test_pos.piece_list[g2] == PIECE_EMPTY && test_pos.piece_list[f4] == WHITE_KNIGHT);
    assert(bitboard_piece_list_consistent(&test_pos));
    assert(test_pos.side_to_move == BLACK);

    unmake_move(m1, &test_pos, move_state_stk);
    //Ensure the captured piece was restored correctly
    assert(test_pos.piece_list[f4] == BLACK_ROOK);
    assert(test_pos.piece_list[g2] == WHITE_KNIGHT);
    assert(bitboard_piece_list_consistent(&test_pos));
    assert(test_pos.side_to_move == WHITE);

    make_move(m2, &test_pos, move_state_stk);
    assert(test_pos.piece_list[g1] == WHITE_KING && test_pos.piece_list[f1] == WHITE_ROOK);
    //Castling means you lose castling right in future moves
    assert(test_pos.can_kingside_castle[WHITE] == false && test_pos.can_queenside_castle[WHITE] == false);
    assert(bitboard_piece_list_consistent(&test_pos));
    assert(test_pos.side_to_move == BLACK);

    unmake_move(m2, &test_pos, move_state_stk);
    assert(test_pos.piece_list[e1] == WHITE_KING && test_pos.piece_list[h1] == WHITE_ROOK);
    assert(test_pos.can_kingside_castle[WHITE] == true && test_pos.can_queenside_castle[WHITE] == true);
    assert(bitboard_piece_list_consistent(&test_pos));
    assert(test_pos.side_to_move == WHITE);

    make_move(m3, &test_pos, move_state_stk);
    assert(test_pos.piece_list[c1] == WHITE_KING && test_pos.piece_list[d1] == WHITE_ROOK);
    assert(test_pos.can_kingside_castle[WHITE] == false && test_pos.can_queenside_castle[WHITE] == false);
    assert(bitboard_piece_list_consistent(&test_pos));
    assert(test_pos.side_to_move == BLACK);

    make_move(m5, &test_pos, move_state_stk);
    assert(test_pos.side_to_move == WHITE);

    make_move(m4, &test_pos, move_state_stk);
    assert(test_pos.piece_list[e5] == PIECE_EMPTY); //Captured pawn
    assert(test_pos.piece_list[d5] == PIECE_EMPTY);
    assert(test_pos.piece_list[e6] == WHITE_PAWN);
    assert(bitboard_piece_list_consistent(&test_pos));
    assert(test_pos.side_to_move == BLACK);

    unmake_move(m4, &test_pos, move_state_stk);
    assert(test_pos.piece_list[e5] == BLACK_PAWN);
    assert(test_pos.piece_list[d5] == WHITE_PAWN);
    assert(test_pos.piece_list[e6] == PIECE_EMPTY);
    assert(bitboard_piece_list_consistent(&test_pos));
    assert(test_pos.side_to_move == WHITE);

    unmake_move(m5, &test_pos, move_state_stk);
    assert(bitboard_piece_list_consistent(&test_pos));
    unmake_move(m3, &test_pos, move_state_stk);
    assert(test_pos.piece_list[e1] == WHITE_KING);
    assert(test_pos.piece_list[a1] == WHITE_ROOK);
}

void test_stack() {
    MS_Stack* stack = stk_create(50);
    assert(stk_empty(stack));
    for (int i = 0; i < 100; ++i)  {
        struct Move_state ms = {0};
        ms.half_move_clock = i; // Use the half move clock to differentiate the different move structs in this test.
        int res = stk_push(stack, ms);
        assert(res == 0);
    }

    for (int j = 99; j >= 0; --j) {
        struct Move_state ms = stk_pop(stack);
        assert(ms.half_move_clock == j);
    }

    assert(stk_empty(stack));
    stk_destroy(stack);
}

void test_legal_move_check() {
    init_LUTs();
    struct Position pos1 = pos_from_FEN("3kr3/8/8/q7/7B/6n1/3PR3/R3K2R w KQkq - 0 1");
    struct Move mov1 = create_regular_move(d2, d4);
    struct Move mov2 = create_regular_move(e2, e6);
    struct Move mov3 = create_regular_move(e2, f2);
    struct Move mov4 = create_special_move(CASTLING, PT_NULL, e1, g1);
    struct Move mov5 = create_special_move(CASTLING, PT_NULL, e1, c1);

    MS_Stack *move_state_stk = stk_create(64);
    assert(!legal(mov1, &pos1, move_state_stk)); //The pawn is pinned
    assert(legal(mov2, &pos1, move_state_stk)); //Rook moves along the same line as the attacker to the king
    assert(!legal(mov3, &pos1, move_state_stk)); //Rook is pinned, and moves so that the king is under attack
    assert(!legal(mov4, &pos1, move_state_stk)); //Kingside castling prohibited since king passes through check
    assert(legal(mov5, &pos1, move_state_stk)); //Queenside castling not blocked.

    struct Position castling_check = pos_from_FEN("4k3/8/8/8/4q3/8/8/R3K2R w KQ - 0 1");
    struct Move ks_castle = create_special_move(CASTLING, PT_NULL, e1, g1);
    struct Move qs_castle = create_special_move(CASTLING, PT_NULL, e1, c1);
    assert(!legal(ks_castle, &castling_check, move_state_stk));
    assert(!legal(qs_castle, &castling_check, move_state_stk));
}

void run_perft_tests() {
    init_LUTs();
    struct Position starting_pos = pos_from_FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    struct Position pos_2 = pos_from_FEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    struct Position pos_3 = pos_from_FEN("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    struct Position pos_4 = pos_from_FEN("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    struct Position pos_5 = pos_from_FEN("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");


    uint64_t starting_pos_results[] = {20, 400, 8902, 197281, 4865609};
    uint64_t pos_2_results[] = {48, 2039, 97862, 4085603, 193690690};
    uint64_t pos_3_results[] = {14, 191, 2812, 43238, 674624};
    uint64_t pos_4_results[] = {6, 264, 9467, 422333, 15833292};
    uint64_t pos_5_results[] = {44, 1486, 62379, 2103487, 89941194};
    MS_Stack *move_state_stk_1 = stk_create(256);
    MS_Stack *move_state_stk_2 = stk_create(256);
    MS_Stack *move_state_stk_3 = stk_create(256);
    MS_Stack *move_state_stk_4 = stk_create(256);
    MS_Stack *move_state_stk_5 = stk_create(256);

    struct Perft_counts cts = {0};
    for (int depth = 1; depth <= 4; ++depth) {
        printf("Depth: %d\n", depth);
        uint64_t starting_res = perft(&starting_pos, depth, move_state_stk_1, &cts, false);
        uint64_t pos_2_res = perft(&pos_2, depth, move_state_stk_2, &cts, false);
        uint64_t pos_3_res = perft(&pos_3, depth, move_state_stk_3, &cts, false);
        uint64_t pos_4_res = perft(&pos_4, depth, move_state_stk_4, &cts, false);
        uint64_t pos_5_res = perft(&pos_5, depth, move_state_stk_5, &cts, false);
        assert(starting_res == starting_pos_results[depth - 1]);
        assert(pos_2_res == pos_2_results[depth - 1]);
        assert(pos_3_res == pos_3_results[depth - 1]);
        assert(pos_4_res == pos_4_results[depth - 1]);
        assert(pos_5_res == pos_5_results[depth - 1]);
    }
    puts("Perft done!");

    stk_destroy(move_state_stk_1);
    stk_destroy(move_state_stk_2);
    stk_destroy(move_state_stk_3);
    stk_destroy(move_state_stk_4);
    stk_destroy(move_state_stk_5);
}

uint64_t perft(struct Position* pos, int depth, MS_Stack *move_state_stk, struct Perft_counts *cts, bool print_divide_info) {
    struct Move move_list[256];
    uint64_t nodes = 0;

    int num_generated_moves = generate_moves(move_list, pos);
    if (depth == 0)
        return 1;
    //u64 king_bb = pos->piece_bb[KING][pos->side_to_move];
    //enum Square king_square = lsb(king_bb);

    for (int i = 0; i < num_generated_moves; ++i) {
        if (move_list[i].castling)
            ++cts->castlings;
        if (move_list[i].en_passant)
            ++cts->en_passants;
        if (move_list[i].promotion_type != PT_NULL)
            ++cts->promotions;

        if (print_divide_info) {
            uint64_t move_cts = 0;
            print_move(move_list[i], pos);
            make_move(move_list[i], pos, move_state_stk);
            move_cts += perft(pos, depth - 1, move_state_stk, cts, false);
            nodes += move_cts;
            unmake_move(move_list[i], pos, move_state_stk);
            printf("Number of sub-leaves: %lu\n\n", move_cts);
        }

        else {
            make_move(move_list[i], pos, move_state_stk);
            nodes += perft(pos, depth - 1, move_state_stk, cts, false);
            unmake_move(move_list[i], pos, move_state_stk);
        }
    }
    return nodes;
}

