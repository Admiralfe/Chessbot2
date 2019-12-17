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
    u64 occupied_squares;

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
};

//Prints a simple ASCII representation of the position.
void print_position(struct Position position);

void make_move(struct Position position, struct Move move);

static inline enum Piece get_piece(struct Position p, enum Square s) {
    return p.piece_list[s];
}

struct Position pos_from_FEN(char *fen_str);
#endif