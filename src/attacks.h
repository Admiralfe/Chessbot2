#ifndef ATTACKS_H
#define ATTACKS_H

#include "types.h"

//First index is color, second square.
struct {
    u64 pawn[2][64];
    u64 knight[64];
    u64 king[64];
} attack_set;

void fill_attack_sets();
#endif