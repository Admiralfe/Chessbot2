#ifndef BITBOARD_H
#define BITBOARD_H

#include <assert.h>
#include <stdbool.h>

#include "types.h"

extern const u64 FileABB;
extern const u64 FileBBB;
extern const u64 FileCBB;
extern const u64 FileDBB;
extern const u64 FileEBB;
extern const u64 FileFBB;
extern const u64 FileGBB;
extern const u64 FileHBB;

extern const u64 Rank1BB;
extern const u64 Rank2BB;
extern const u64 Rank3BB;
extern const u64 Rank4BB;
extern const u64 Rank5BB;
extern const u64 Rank6BB;
extern const u64 Rank7BB;
extern const u64 Rank8BB;

extern const u64 Main_diagBB;
extern const u64 Anti_diagBB;

void print_bitboard(u64 bitboard);

u64 shift_bb(u64 bitboard, enum Direction dir);

static inline u64 shift_N(u64 bitboard) { return bitboard << 8; }
static inline u64 shift_S(u64 bitboard) { return bitboard >> 8; }
static inline u64 shift_E(u64 bitboard) { return (bitboard & ~FileHBB) << 1; }
static inline u64 shift_W(u64 bitboard) { return (bitboard & ~FileABB) >> 1; }
static inline u64 shift_NE(u64 bitboard) { return (bitboard & ~FileHBB) << 9; }
static inline u64 shift_SE(u64 bitboard) { return (bitboard & ~FileHBB) >> 7; }
static inline u64 shift_NW(u64 bitboard) { return (bitboard & ~FileABB) << 7; }
static inline u64 shift_SW(u64 bitboard) { return (bitboard & ~FileABB) >> 9; }

static inline u64 set_bit(enum Square s) { return 1ULL << s; }

static inline bool is_set(enum Square s, u64 bitboard) { return (bitboard & (set_bit(s))) != 0; }

#if defined(__GNUC__) //GCC, Clang, ICC

static inline enum Square lsb(u64 bb) {
    assert(bb); //At least one bit must be set
    return (enum Square) __builtin_ctzll(bb);
}

static inline enum Square msb(u64 bb) {
    assert(bb); //At least one bit must be set
    return ((enum Square) (63U ^ (unsigned) __builtin_clzll(bb)));
}

#elif defined(_MSC_VER) //MSVC

static inline enum Square lsb(u64 bb) {
    assert(bb); //At least one bit must be set
    unsigned long idx;
    _BitScanForward64(&idx, b);
    return (enum Square) idx;
}


static inline enum Square msb(u64 bb) {
    assert(bb); //At least one bit must be set
    unsigned long idx;
    _BitScanReverse64(&idx, b);
    return (enum Square) idx;
}

#endif //GCC defined

static inline enum Square pop_lsb(u64 *bb) {
    enum Square sq = lsb(*bb);
    *bb &= *bb - 1;
    return sq;
}

//Populates a list of squares corresponding to set bits in the given bitboard.
void get_squares(enum Square *sq_list, u64 bb);

#endif //HEADER GUARD