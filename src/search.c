#include <stdlib.h>

#include "search.h"
#include "position.h"
#include "evaluation.h"
#include "movegen.h"

#define MAX_NUM_MOVES 256

void negamax_root(struct Position *pos, int depth) {

}

int negamax(struct Position *pos, int depth, int alpha, int beta) {

    int sign = pos->side_to_move == WHITE ? 1 : -1;
    if (depth == 0)
        return sign * evaluate_position(pos, pos->side_to_move);

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