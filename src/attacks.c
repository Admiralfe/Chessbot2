#include <stdio.h>

#include "attacks.h"
#include "bitboard.h"
#include "types.h"

static u64 knight_attacks(enum Square sq) {
    u64 bb = set_bit(sq);
    return ((bb & ~FileHBB) << 17) //North North East
         | ((bb & ~(FileHBB | FileGBB)) << 10) // North East East
         | ((bb & ~(FileHBB | FileGBB)) >> 6) //South East East
         | ((bb & ~FileHBB) >> 15) //South South East
         | ((bb & ~FileABB) << 15) //North North West
         | ((bb & ~(FileABB | FileBBB)) << 6) //North West West
         | ((bb & ~(FileABB | FileBBB)) >> 10) //South West West
         | ((bb & ~FileABB) >> 17); //South South West
}

static u64 king_attacks(enum Square sq) {
    u64 bb = set_bit(sq);
    return shift_N(bb) | shift_NE(bb & ~FileHBB) | shift_E(bb & ~FileHBB)
         | shift_SE(bb & ~FileHBB)| shift_S(bb)  | shift_SW(bb & ~FileABB)
         | shift_W(bb & ~FileABB) | shift_NW(bb & ~FileABB);
}

void fill_attack_rays() {
    //Start with north and south rays, since we don't have to worry about wraps in this case
    u64 north_ray = FileABB ^ set_bit(a1);
    u64 south_ray = FileHBB ^ set_bit(h8);

    fprintf(stderr, "Filling north and south attack rays...");
    for (int sq = 0; sq < 64; ++sq) {
        attack_rays[sq][NORTH] = north_ray;
        north_ray <<= 1;
    }

    for (int sq = 63; sq >= 0; --sq) {
        attack_rays[sq][SOUTH] = south_ray;
        south_ray >>= 1;
    }
    fprintf(stderr, "Done!\n");

    //Northeast & east & southeast now, taking care because of wraps
    u64 noea_ray = Main_diagBB ^ set_bit(a1);
    u64 soea_ray = Anti_diagBB ^ set_bit(a8);
    u64 ea_ray = Rank1BB ^ set_bit(a1);

    fprintf(stderr, "Filling NE, E and SE attack rays...");
    for (int file = 0; file < 8; ++file) {
        u64 tmp_ne_ray = noea_ray;
        u64 tmp_se_ray = soea_ray;
        u64 tmp_e_ray = ea_ray;
        
        //NE and E
        for (int rank8 = a1; rank8 < 64; rank8 += 8) {
            attack_rays[rank8 + file][NORTHEAST] = tmp_ne_ray;
            tmp_ne_ray = shift_N(tmp_ne_ray);

            attack_rays[rank8 + file][EAST] = tmp_e_ray;
            tmp_e_ray = shift_N(tmp_e_ray);
        }

        //SE
        for (int rank8 = a8; rank8 >= 0; rank8 -= 8) {
            attack_rays[rank8 + file][SOUTHEAST] = tmp_se_ray;
            tmp_se_ray = shift_S(tmp_se_ray);
        }
        
        noea_ray = shift_E(noea_ray);
        soea_ray = shift_E(soea_ray);
        ea_ray = shift_E(ea_ray);
    }
    fprintf(stderr, "Done!\n");

    u64 nowe_ray = Anti_diagBB ^ set_bit(h1);
    u64 sowe_ray = Main_diagBB ^ set_bit(h8);
    u64 we_ray = Rank1BB ^ set_bit(h1);

    fprintf(stderr, "Filling NW, W and SW attack rays...");
    //Northwest & west &  Southwest
    for (int file = 7; file >= 0; --file) {
        u64 tmp_nw_ray = nowe_ray;
        u64 tmp_sw_ray = sowe_ray;
        u64 tmp_w_ray = we_ray;

        //NW and W
        for (int rank8 = a1; rank8 < 64; rank8 += 8) {
            attack_rays[rank8 + file][NORTHWEST] = tmp_nw_ray;
            tmp_nw_ray = shift_N(tmp_nw_ray);

            attack_rays[rank8 + file][WEST] = tmp_w_ray;
            tmp_w_ray = shift_N(tmp_w_ray);
        }

        //SW
        for (int rank8 = a8; rank8 >= 0; rank8 -= 8) {
            attack_rays[rank8 + file][SOUTHWEST] = tmp_sw_ray;
            tmp_sw_ray = shift_S(tmp_sw_ray);
        }

        nowe_ray = shift_W(nowe_ray);
        sowe_ray = shift_W(sowe_ray);
        we_ray = shift_W(we_ray);
    }
    fprintf(stderr, "Done!\n");
}

void fill_attack_sets() {
    for (int sq = 0; sq < 64; ++sq) {
        attack_set.pawn[WHITE][sq] = shift_NE(set_bit(sq)) | shift_NW(set_bit(sq));
        attack_set.pawn[BLACK][sq] = shift_SE(set_bit(sq)) | shift_SW(set_bit(sq));

        attack_set.knight[sq] = knight_attacks(sq);
        attack_set.king[sq] = king_attacks(sq);
    }
}

u64 positive_ray_attacks(u64 occupancy, enum Direction dir, enum Square sq) {
    assert(dir == EAST || dir == NORTH || dir == NORTHEAST || dir == NORTHWEST);

    u64 attacks = attack_rays[sq][dir];
    u64 blockers = occupancy & attacks;
    enum Square blocker_sq;
    if (blockers) {
        blocker_sq = lsb(blockers);
        attacks ^= attack_rays[blocker_sq][dir];
    }

    return attacks;
}

u64 negative_ray_attacks(u64 occupancy, enum Direction dir, enum Square sq) {
    assert(dir == WEST || dir == SOUTH || dir == SOUTHEAST || dir == SOUTHWEST);

    u64 attacks = attack_rays[sq][dir];
    u64 blockers = occupancy & attacks;
    enum Square blocker_sq;
    if (blockers) {
        blocker_sq = msb(blockers);
        attacks ^= attack_rays[blocker_sq][dir];
    }

    return attacks;
}

u64 attacks_from(enum Piece_type piece_type, enum Square sq, u64 occupancy) {
    switch (piece_type) {
        case ROOK:
            return positive_ray_attacks(occupancy, NORTH, sq) |
                positive_ray_attacks(occupancy, EAST, sq)  |
                negative_ray_attacks(occupancy, SOUTH, sq) |
                negative_ray_attacks(occupancy, WEST, sq);
        case BISHOP:
            return positive_ray_attacks(occupancy, NORTHEAST, sq) |
                positive_ray_attacks(occupancy, NORTHWEST, sq)  |
                negative_ray_attacks(occupancy, SOUTHEAST, sq) |
                negative_ray_attacks(occupancy, SOUTHWEST, sq);

        case QUEEN:
            return attacks_from(BISHOP, sq, occupancy) |
                   attacks_from(ROOK, sq, occupancy);
        
        case KING:
            return king_attacks(sq);
        
        case KNIGHT:
            return knight_attacks(sq);

        default:
            return 0ULL;
    }
}

