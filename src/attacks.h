#ifndef ATTACKS_H
#define ATTACKS_H

#include "types.h"

//First index is color, second square.
struct {
    u64 pawn[2][64];
    u64 knight[64];
    u64 king[64];
} attack_set;

//Pre-calculated attack rays in 8 directions (N, NE, E, SE etc.) for each of the 64 squares.
u64 attack_rays[64][8];

void fill_attack_rays();
void fill_attack_sets();
#endif