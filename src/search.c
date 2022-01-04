#include <stdlib.h>
#include <stdio.h>

#include "search.h"
#include "position.h"
#include "evaluation.h"
#include "movegen.h"

#define MAX_NUM_MOVES 256

int max(int a, int b) {
    return a > b ? a : b;
};

int negamax_root(struct Position *pos, int depth, struct Move *best_move) {
    struct Move* move_list = malloc(MAX_NUM_MOVES * sizeof(struct Move));
    int num_legal_moves = generate_moves(move_list, pos);
    int values[num_legal_moves];
    MS_Stack *move_stack = stk_create(16);
    for (int i = 0; i < num_legal_moves; ++i) {
        make_move(move_list[i], pos, move_stack);
        values[i] = -negamax(pos, depth - 1, -1000000, 1000000);
        unmake_move(move_list[i], pos, move_stack);
    }
    int max_val = -100000000;
    int max_index = 0;
    for (int i = 0; i < num_legal_moves; ++i) {
        if (values[i] > max_val) {
            max_val = values[i];
            *best_move = move_list[i];
        }
    }
    free(move_list);
    return max_val;
}

int negamax(struct Position *pos, int depth, int alpha, int beta) {
    if (depth == 0)
        return evaluate_position(pos, pos->side_to_move);

    struct Move* move_list = malloc(MAX_NUM_MOVES * sizeof(struct Move));

    int num_legal_moves = generate_moves(move_list, pos);
    int value = -10000;
    MS_Stack *move_stack = stk_create(16);
    for (int i = 0; i < num_legal_moves; ++i) {
        make_move(move_list[i], pos, move_stack);
        value = max(value, -negamax(pos, depth - 1, -beta, -alpha));
        unmake_move(move_list[i], pos, move_stack);
        alpha = max(alpha, value);
        if (alpha >= beta)
            break;
    }

    stk_destroy(move_stack);
    free(move_list);
    return value;
}