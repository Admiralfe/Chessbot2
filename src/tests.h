#ifndef TESTS_H
#define TESTS_H

#include <stdint.h>
#include "stack.h"
struct Position;

void run_all_tests(void);

void test_FEN(void);
void test_bitshifts(void);
void test_attack_sets(void);
void test_ray_attacks(void);
void test_attacks_from(void);
void test_movegen_pawns(void);
void test_movegen(void);
void test_castling_rights(void);
void test_in_between_LUT(void);
void test_attackers_to(void);
void test_make_move(void);
void test_stack(void);
void test_legal_move_check(void);
void run_perft_tests(void);
void test_evaluate_position(void);

struct Perft_counts {
    int checkmates;
    int castlings;
    int en_passants;
    int promotions;
};

uint64_t perft(struct Position *start_pos, int depth, MS_Stack *move_state_stk, struct Perft_counts *cts, bool print_divide_info);

#endif
