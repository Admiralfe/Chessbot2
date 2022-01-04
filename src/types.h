#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

//We use 64-bit words to represent the boards
typedef uint64_t u64;

enum Square {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8,
    SQUARE_EMPTY = 65
};

//Used to convert from the square number to the squares name in algebraic notation.
extern const char* square_name_LUT[];
extern const char* piece_name_LUT[];
extern const char* piece_type_name_LUT[];

enum Rank {
    RANK_1, RANK_2, RANK_3, RANK_4,
    RANK_5, RANK_6, RANK_7, RANK_8
};

enum File {
    FILE_A, FILE_B, FILE_C, FILE_D,
    FILE_E, FILE_F, FILE_G, FILE_H
};

enum Side { WHITE, BLACK };

enum Piece {
    PIECE_EMPTY,
    WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
    BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING
};

enum Move_type {
    NORMAL, PROMOTION, ENPASSANT, CASTLING
};

enum Piece_type {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, PT_NULL
};

enum Direction {
    NORTH, NORTHEAST, EAST, SOUTHEAST, SOUTH, SOUTHWEST, WEST, NORTHWEST
};

static inline enum Piece_type to_piece_type(enum Piece p) {
    return (p == PIECE_EMPTY) ? PT_NULL : (p - 1) % 6;
}

static inline enum Piece to_colored_piece(enum Piece_type pt, enum Side color) {
    const int offset = (color == WHITE) ? 1 : 7;
    return (pt == PT_NULL) ? PIECE_EMPTY : pt + offset;
}

static inline enum Side piece_color(enum Piece p) {
    return (p <= WHITE_KING) ? WHITE : BLACK;
}

#endif