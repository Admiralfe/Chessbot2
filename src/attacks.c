#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "attacks.h"
#include "bitboard.h"
#include "position.h"
#include "tables.h"
#include "types.h"

u64 attackers_to(enum Square sq, const struct Position *pos, enum Side us) {
    const enum Side them = abs(us - 1);
    u64 attackers = 0ULL;

    attackers |= (attack_set.pawn[us][sq] & pos->piece_bb[PAWN][them]);
    attackers |= (attack_set.knight[sq] & pos->piece_bb[KNIGHT][them]);
    attackers |= (attack_set.king[sq] & pos->piece_bb[KING][them]);
    attackers |= (attacks_from(ROOK, pos, sq) & pos->piece_bb[ROOK][them]);
    attackers |= (attacks_from(BISHOP, pos, sq) & pos->piece_bb[BISHOP][them]);
    attackers |= (attacks_from(QUEEN, pos, sq) & pos->piece_bb[QUEEN][them]);
    attackers &= pos->occupied_squares[them];
    return attackers;
}

u64 knight_attacks(enum Square sq) {
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

u64 king_attacks(enum Square sq) {
    u64 bb = set_bit(sq);
    return shift_N(bb) | shift_NE(bb & ~FileHBB) | shift_E(bb & ~FileHBB)
         | shift_SE(bb & ~FileHBB)| shift_S(bb)  | shift_SW(bb & ~FileABB)
         | shift_W(bb & ~FileABB) | shift_NW(bb & ~FileABB);
}

u64 positive_ray_attacks(u64 occupancy, enum Direction dir, enum Square sq) {
    assert(dir == EAST || dir == NORTH || dir == NORTHEAST || dir == NORTHWEST);

    u64 attacks = (*attack_rays_ptr)[sq][dir];
    u64 blockers = occupancy & attacks;
    enum Square blocker_sq;
    if (blockers) {
        blocker_sq = lsb(blockers);
        attacks ^= (*attack_rays_ptr)[blocker_sq][dir];
    }

    return attacks;
}

u64 negative_ray_attacks(u64 occupancy, enum Direction dir, enum Square sq) {
    assert(dir == WEST || dir == SOUTH || dir == SOUTHEAST || dir == SOUTHWEST);

    u64 attacks = (*attack_rays_ptr)[sq][dir];
    u64 blockers = occupancy & attacks;
    enum Square blocker_sq;
    if (blockers) {
        blocker_sq = msb(blockers);
        attacks ^= (*attack_rays_ptr)[blocker_sq][dir];
    }

    return attacks;
}

u64 attacks_from(enum Piece_type pt, const struct Position *pos, enum Square sq) {
    u64 occupancy = pos_occupancy(pos);
    switch (pt) {
        case ROOK:
            return positive_ray_attacks(occupancy, NORTH, sq) |
                positive_ray_attacks(occupancy, EAST, sq)  |
                negative_ray_attacks(occupancy, SOUTH, sq) |
                negative_ray_attacks(occupancy, WEST, sq);
        case BISHOP:
            return positive_ray_attacks(occupancy, NORTHEAST, sq) |
                positive_ray_attacks(occupancy, NORTHWEST, sq) |
                negative_ray_attacks(occupancy, SOUTHEAST, sq) |
                negative_ray_attacks(occupancy, SOUTHWEST, sq);

        case QUEEN:
            return attacks_from(BISHOP, pos, sq) |
                   attacks_from(ROOK, pos, sq);

        case KING:
            return king_attacks(sq);

        case KNIGHT:
            return knight_attacks(sq);

        default:
            return 0ULL;
    }
}

