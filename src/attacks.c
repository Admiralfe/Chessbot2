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

void fill_attack_sets() {
    for (int sq = 0; sq < 64; ++sq) {
        attack_set.pawn[WHITE][sq] = shift_NE(set_bit(sq)) | shift_NW(set_bit(sq));
        attack_set.pawn[BLACK][sq] = shift_SE(set_bit(sq)) | shift_SW(set_bit(sq));

        attack_set.knight[sq] = knight_attacks(sq);
        attack_set.king[sq] = king_attacks(sq);
    }
}