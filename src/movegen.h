#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "position.h"
#include "types.h"

int generate_pawn_moves(struct Move *move_list, struct Position pos);
int generate_moves_pt(enum Piece_type pt, struct Move *move_list, const struct Position *pos);

int generate_moves(struct Move *move_list, struct Position pos);

#endif
