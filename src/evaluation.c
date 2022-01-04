#include <stdlib.h>

#include "bitboard.h"
#include "evaluation.h"
#include "position.h"
#include "types.h"

//Indexing this array by Piece_type gives the value in centipawns for that piece
static const int piece_value[] = {100, 300, 300, 500, 900, 100000, 0};

int evaluate_position(const struct Position *pos, enum Side side) {
    int score = 0;
    //Material count in centipawns
    for (enum Piece_type pt = PAWN; pt <= KING; ++pt) {
        u64 white_bitboard = pos->piece_bb[pt][WHITE];
        while (white_bitboard) {
            score += piece_value[pt];
            pop_lsb(&white_bitboard);
        }
        u64 black_bitboard = pos->piece_bb[pt][BLACK];
        while (black_bitboard) {
            score -= piece_value[pt];
            pop_lsb(&black_bitboard);
        }
    }

    int factor = side == WHITE ? 1 : -1;

    return factor * score;
}