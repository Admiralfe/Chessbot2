#ifndef POSITION_H
#define POSITION_H

#include <stdbool.h>

#include "types.h"

struct Position {
    /* Piece information and square information */
    u64 pawns[2];
    u64 knights[2];
    u64 bishops[2];
    u64 rooks[2];
    u64 queens[2];
    u64 kings[2];

    u64 empty_squares;

    //occupied squares indexed by color
    u64 occupied_squares[2];

    enum Piece piece_list[64];
    
    /* state related information */
    enum Side side_to_move;

    bool can_kingside_castle[2];
    bool can_queenside_caslte[2];

    enum Square ep_square;

    unsigned int half_move_clock;
    unsigned int fullmove_count;
    
};

struct Move {
    enum Square from_sq;
    enum Square to_sq;

    enum Piece_type promotion_type;
    bool en_passant;
    bool castling;
};

struct Move create_regular_move(enum Square from, enum Square to);
struct Move create_special_move(enum Move_type type, enum Piece_type piece_type, enum Square from, enum Square to);

//Prints a move in the given notation in algebraic notation.
void print_move(struct Move move, struct Position pos);

//Prints a simple ASCII representation of the position.
void print_position(struct Position position);

static inline enum Piece get_piece(struct Position p, enum Square s) {
    return p.piece_list[s];
}

static inline u64 pos_occupancy(struct Position pos) {
    return pos.occupied_squares[WHITE] | pos.occupied_squares[BLACK];
}

struct Position pos_from_FEN(char *fen_str);
#endif