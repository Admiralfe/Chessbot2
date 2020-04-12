#include <stdio.h>

#include "bitboard.h"
#include "types.h"

const u64 FileABB  = 0x0101010101010101ULL;
const u64 FileBBB  = 0x0101010101010101ULL << 1;
const u64 FileCBB  = 0x0101010101010101ULL << 2;
const u64 FileDBB  = 0x0101010101010101ULL << 3;
const u64 FileEBB  = 0x0101010101010101ULL << 4;
const u64 FileFBB  = 0x0101010101010101ULL << 5;
const u64 FileGBB  = 0x0101010101010101ULL << 6;
const u64 FileHBB  = 0x0101010101010101ULL << 7;

const u64 Rank1BB = 0xffULL;
const u64 Rank2BB = 0xffULL << (8 * 1);
const u64 Rank3BB = 0xffULL << (8 * 2);
const u64 Rank4BB = 0xffULL << (8 * 3);
const u64 Rank5BB = 0xffULL << (8 * 4);
const u64 Rank6BB = 0xffULL << (8 * 5);
const u64 Rank7BB = 0xffULL << (8 * 6);
const u64 Rank8BB = 0xffULL << (8 * 7);

const u64 Main_diagBB = 0x8040201008040201ULL;
const u64 Anti_diagBB = 0x0102040810204080ULL;


void print_bitboard(u64 bitboard) {
    char bb_str[73]; //Size is 64 squares, 8 newlines and 1 null terminator
    
    //Index to keep track of position in string
    int i = 0;
    for (int row = 7; row >= 0; --row) {
        for (int col = 0; col < 8; ++col) {
            bb_str[i] = is_set(8*row + col, bitboard) ? '1' : '.';
            ++i;
        }
        bb_str[i] = '\n';
        ++i;
    }   


    bb_str[i] = '\0';

    printf("%s\n", bb_str);
}

void get_squares(enum Square *sq_list, u64 bb) {
    while (bb) 
        *sq_list++ = pop_lsb(&bb);
}

u64 shift_bb(u64 bitboard, enum Direction dir) {
    switch (dir) {
        case NORTH:
            return shift_N(bitboard);
        case SOUTH:
            return shift_S(bitboard);
        case NORTHEAST:
            return shift_NE(bitboard);
        case EAST:
            return shift_E(bitboard);
        case SOUTHEAST:
            return shift_SE(bitboard);
        case SOUTHWEST:
            return shift_SW(bitboard);
        case WEST:
            return shift_W(bitboard);
        case NORTHWEST:
            return shift_NW(bitboard);
    }

    return 0ULL;
}