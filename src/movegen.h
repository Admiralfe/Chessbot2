#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "position.h"
#include "types.h"

int generate_pawn_moves(struct Move *move_list, struct Position pos);
void generate_moves(struct Move *move_list, struct Position pos);

#endif