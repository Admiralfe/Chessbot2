#ifndef TABLES_H
#define TABLES_H

#include "types.h"

//First index is color, second square.
struct Attack_set {
    u64 pawn[2][64];
    u64 knight[64];
    u64 king[64];
};

extern struct Attack_set attack_set;

extern const u64 (*const attack_rays_ptr)[64][8];
extern const u64 (*const in_between_LUT_ptr)[64][64];

void init_LUTs(void);

void fill_inbetween_LUT(void);
void fill_attack_rays(void);
void fill_attack_sets(void);


#endif
