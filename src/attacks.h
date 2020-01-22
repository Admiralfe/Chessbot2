#ifndef ATTACKS_H
#define ATTACKS_H

#include "types.h"

u64 knight_attacks(enum Square sq);
u64 king_attacks(enum Square sq);

u64 positive_ray_attacks(u64 occupancy, enum Direction dir, enum Square sq);
u64 negative_ray_attacks(u64 occupancy, enum Direction dir, enum Square sq);


struct Position;
u64 attacks_from(enum Piece_type pt, const struct Position *pos, enum Square sq);
#endif
