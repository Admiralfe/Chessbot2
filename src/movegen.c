#include "attacks.h"
#include "bitboard.h"
#include "movegen.h"
#include "position.h"
#include "types.h"

int generate_pawn_moves(struct Move* move_list, struct Position pos) {
    int number_of_moves = 0;

    enum Side us = pos.side_to_move;
    enum Side them = (us == WHITE) ? BLACK : WHITE;
    enum Direction up = (us == WHITE) ? NORTH : SOUTH;
    enum Direction down = (us == WHITE) ? SOUTH : NORTH;

    u64 our_rank7bb = (us == WHITE) ? Rank7BB : Rank2BB;
    u64 our_rank3bb = (us == WHITE) ? Rank3BB : Rank6BB;

    u64 pawns = pos.pawns[us];
    u64 promotion_pawns = pawns & our_rank7bb;
    u64 promotion_targets = shift_bb(promotion_pawns, up) & ~pos.occupied_squares[us];

    u64 non_promotion_pawns = pawns ^ promotion_pawns;
    u64 single_push_targets = shift_bb(non_promotion_pawns, up) & ~pos.occupied_squares[us];
    u64 double_push_targets = shift_bb(single_push_targets & our_rank3bb, up) & ~pos.occupied_squares[us];

    while (promotion_pawns) {
        enum Square from = pop_lsb(&promotion_pawns);
        //Attacked squares
        u64 attacked_sqs = attack_set.pawn[us][from] & pos.occupied_squares[them];
        while (attacked_sqs) {
            enum Square to = pop_lsb(&attacked_sqs);

            //Create the promotion moves
            *move_list++ = create_special_move(PROMOTION, KNIGHT, from, to);
            *move_list++ = create_special_move(PROMOTION, BISHOP, from, to);
            *move_list++ = create_special_move(PROMOTION, ROOK, from, to);
            *move_list++ = create_special_move(PROMOTION, QUEEN, from, to);
            number_of_moves += 4;
        }
    }

    while (promotion_targets) {
        enum Square to = pop_lsb(&promotion_targets);
        enum Square from  = (us == WHITE) ? to - 8 : to + 8;

        *move_list++ = create_special_move(PROMOTION, KNIGHT, from, to);
        *move_list++ = create_special_move(PROMOTION, BISHOP, from, to);
        *move_list++ = create_special_move(PROMOTION, ROOK, from, to);
        *move_list++ = create_special_move(PROMOTION, QUEEN, from, to);
        number_of_moves += 4;
    }

    //Regular pawn attacks
    while (non_promotion_pawns) {
        enum Square from = pop_lsb(&non_promotion_pawns);
        u64 attacked_sqs = attack_set.pawn[us][from] & pos.occupied_squares[them];

        while (attacked_sqs) {
            enum Square to = pop_lsb(&attacked_sqs);
            *move_list++ = create_regular_move(from, to);
            ++number_of_moves;
        }
    }

    while (single_push_targets) {
        enum Square to = pop_lsb(&single_push_targets);
        enum Square from = (us == WHITE) ? to - 8 : to + 8;

        *move_list++ = create_regular_move(from, to);
        ++number_of_moves;
    }

    while (double_push_targets) {
        enum Square to = pop_lsb(&double_push_targets);
        enum Square from = (us == WHITE) ? to - 16 : to + 16;

        *move_list++ = create_regular_move(from, to);
        ++number_of_moves;
    }

    //Enpassant moves
    if (pos.ep_square != SQUARE_EMPTY) {
        printf("%d\n", pos.ep_square);
        u64 ep_square_bb = set_bit(pos.ep_square);
        u64 ep_attacker_squares = shift_E(shift_bb(ep_square_bb, down)) | shift_W(shift_bb(ep_square_bb, down));

        print_bitboard(ep_square_bb);
        print_bitboard(ep_attacker_squares);

        //Opponent made double pawn push last move.
        //Check if any of our pawns attack the target square.
        u64 ep_attackers = pos.pawns[us] & ep_attacker_squares;

        while (ep_attackers) {
            enum Square from = pop_lsb(&ep_attackers);
            *move_list++ = create_special_move(ENPASSANT, PT_NULL, from, pos.ep_square);
            ++number_of_moves;
        }
    }

    return number_of_moves;
}