#include <assert.h>
#include <stdio.h>

#include "tests.h"
#include "bitboard.h"
#include "position.h"
#include "types.h"

int main(void) {
    //test_movegen();
    run_perft_tests();
    /* 
    run_all_tests();
    MS_Stack *move_state_stk = stk_create(256);
    struct Perft_counts cts = {0};
    struct Position pos_2 = pos_from_FEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    struct Move m = create_regular_move(a1, b1);
    MS_Stack *tmp = stk_create(16);
    make_move(m, &pos_2, tmp);
    //make_move(m2, &pos_2, tmp);
    printf("%d\n", pos_2.piece_list[e8]);
    print_bitboard(pos_2.piece_bb[KING][WHITE]);
    print_bitboard(pos_2.piece_bb[ROOK][WHITE]);
    uint64_t perft_cts = perft(&pos_2, 4, move_state_stk, &cts, true);
    */
    
    //printf("Number of castling moves: %d\n", cts.castlings);
    //printf("Number of en passant moves: %d\n", cts.en_passants);
    //printf("Number of promotions: %d\n", cts.promotions);
    //printf("Number of checkmates: %d\n", cts.checkmates);
    //printf("Number of moves: %u\n", perft_cts);
    
    //run_all_tests();
    //test_castling_rights();
    //test_attackers_to();
    //test_movegen_pawns();
    //test_make_move();
    //test_legal_move_check();
    return 0;
}