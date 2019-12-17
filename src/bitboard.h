#ifndef BITBOARD_H
#define BITBOARD_H

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

void print_bitboard(u64 bitboard);

static inline u64 shift_N(u64 bitboard) { return bitboard << 8; }
static inline u64 shift_S(u64 bitboard) { return bitboard >> 8; }
static inline u64 shift_E(u64 bitboard) { return (bitboard & ~FileHBB) << 1; }
static inline u64 shift_W(u64 bitboard) { return (bitboard & ~FileABB) >> 1; }
static inline u64 shift_NE(u64 bitboard) { return (bitboard & ~FileHBB) << 9; }
static inline u64 shift_SE(u64 bitboard) { return (bitboard & ~FileHBB) >> 7; }
static inline u64 shift_NW(u64 bitboard) { return (bitboard & ~FileABB) << 7; }
static inline u64 shift_SW(u64 bitboard) { return (bitboard & ~FileABB) >> 9; }

static inline u64 set_bit(enum Square s) { return 1ULL << s; }

static inline bool is_set(u64 bitboard, enum Square s) { return (bitboard & (1ULL << s)) != 0; }

#endif