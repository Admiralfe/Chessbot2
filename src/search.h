#ifndef SEARCH_H
#define SEARCH_H

#include "position.h"

int negamax_root(struct Position *pos, int depth, struct Move *best_move);
int negamax(struct Position *pos, int depth, int alpha, int beta);

#endif
