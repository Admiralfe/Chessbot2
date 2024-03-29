#include <stdio.h>

#include "attacks.h"
#include "bitboard.h"
#include "movegen.h"
#include "position.h"
#include "tables.h"
#include "types.h"

int generate_pawn_moves(struct Move* move_list, struct Position *pos) {
    int number_of_moves = 0;

    enum Side us = pos->side_to_move;
    enum Side them = (us == WHITE) ? BLACK : WHITE;
    enum Direction up = (us == WHITE) ? NORTH : SOUTH;
    enum Direction down = (us == WHITE) ? SOUTH : NORTH;

    u64 our_rank7bb = (us == WHITE) ? Rank7BB : Rank2BB;
    u64 our_rank3bb = (us == WHITE) ? Rank3BB : Rank6BB;

    u64 pawns = pos->piece_bb[PAWN][us];
    u64 promotion_pawns = pawns & our_rank7bb;

    u64 promotion_targets = shift_bb(promotion_pawns, up) & ~pos_occupancy(pos);

    u64 non_promotion_pawns = pawns ^ promotion_pawns;
    u64 single_push_targets = shift_bb(non_promotion_pawns, up) & ~pos_occupancy(pos);
    u64 double_push_targets = shift_bb(single_push_targets & our_rank3bb, up) & ~pos_occupancy(pos);

    MS_Stack *tmp_stack = stk_create(16);
    while (promotion_pawns) {
        enum Square from = pop_lsb(&promotion_pawns);
        //Attacked squares
        u64 attacked_sqs = attack_set.pawn[us][from] & pos->occupied_squares[them];
        while (attacked_sqs) {
            enum Square to = pop_lsb(&attacked_sqs);

            struct Move promotion_move = create_special_move(PROMOTION, KNIGHT, from, to);

            //If one of the promotion moves is legal, all of them must be.
            if (legal(promotion_move, pos, tmp_stack)) {
            //Create the promotion moves
                *move_list++ = promotion_move;
                *move_list++ = create_special_move(PROMOTION, BISHOP, from, to);
                *move_list++ = create_special_move(PROMOTION, ROOK, from, to);
                *move_list++ = create_special_move(PROMOTION, QUEEN, from, to);
                number_of_moves += 4;
            }
        }
    }

    while (promotion_targets) {
        enum Square to = pop_lsb(&promotion_targets);
        enum Square from  = (us == WHITE) ? to - 8 : to + 8;
        //If one of the promotion moves is legal, all of them are.
        struct Move promotion_move = create_special_move(PROMOTION, KNIGHT, from, to);
        if (legal(promotion_move, pos, tmp_stack)) {
            *move_list++ = promotion_move;
            *move_list++ = create_special_move(PROMOTION, BISHOP, from, to);
            *move_list++ = create_special_move(PROMOTION, ROOK, from, to);
            *move_list++ = create_special_move(PROMOTION, QUEEN, from, to);
            number_of_moves += 4;
        }
    }

    //Regular pawn attacks
    while (non_promotion_pawns) {
        enum Square from = pop_lsb(&non_promotion_pawns);
        u64 attacked_sqs = attack_set.pawn[us][from] & pos->occupied_squares[them];

        while (attacked_sqs) {
            enum Square to = pop_lsb(&attacked_sqs);
            struct Move new_move = create_regular_move(from, to);

            if (legal(new_move, pos, tmp_stack)) {
                *move_list++ = new_move;
                ++number_of_moves;
            }
        }
    }

    while (single_push_targets) {
        enum Square to = pop_lsb(&single_push_targets);
        enum Square from = (us == WHITE) ? to - 8 : to + 8;

        struct Move new_move = create_regular_move(from, to);
        if (legal(new_move, pos, tmp_stack)) {
            *move_list++ = new_move;
            ++number_of_moves;
        }
    }

    while (double_push_targets) {
        enum Square to = pop_lsb(&double_push_targets);
        enum Square from = (us == WHITE) ? to - 16 : to + 16;

        struct Move new_move = create_regular_move(from, to);
        if (legal(new_move, pos, tmp_stack)) {
            *move_list++ = new_move;
            ++number_of_moves;
        }
    }

    //Enpassant moves
    if (pos->ep_square != SQUARE_EMPTY) {
        u64 ep_square_bb = set_bit(pos->ep_square);
        u64 ep_attacker_squares = shift_E(shift_bb(ep_square_bb, down)) | shift_W(shift_bb(ep_square_bb, down));

        //Opponent made double pawn push last move.
        //Check if any of our pawns attack the target square.
        u64 ep_attackers = pos->piece_bb[PAWN][us] & ep_attacker_squares;

        while (ep_attackers) {
            enum Square from = pop_lsb(&ep_attackers);
            struct Move new_ep_move = create_special_move(ENPASSANT, PT_NULL, from, pos->ep_square);

            if (legal(new_ep_move, pos, tmp_stack)) {
                *move_list++ = new_ep_move;
                ++number_of_moves;
            }
        }
    }

    stk_destroy(tmp_stack);

    return number_of_moves;
}

int generate_moves_pt(enum Piece_type pt, struct Move *move_list, struct Position *pos) {
    enum Side us = pos->side_to_move;
    u64 piece_squares = pos->piece_bb[pt][us];
    MS_Stack *tmp_stack = stk_create(16);

    int num_moves_added = 0;
    while (piece_squares) {
        enum Square from = pop_lsb(&piece_squares);
        u64 attacked_sqs = attacks_from(pt, pos, from) & ~pos->occupied_squares[us];

        while (attacked_sqs) {
            enum Square to = pop_lsb(&attacked_sqs);
            struct Move new_move = create_regular_move(from, to);
            if (legal(new_move, pos, tmp_stack)) {
                *move_list++ = new_move;
                ++num_moves_added;
            }
        }
    }

    //Check for castling
    if (pt == KING) {
        if (can_kingside_castle(us, pos)) {
            enum Square from = (us == WHITE) ? e1 : e8;
            enum Square to = (us == WHITE) ? g1 : g8;
            struct Move new_move = create_special_move(CASTLING, PT_NULL, from, to);
            if (legal(new_move, pos, tmp_stack)) {
                *move_list++ = new_move;
                ++num_moves_added;
            }
        }

        if (can_queenside_castle(us, pos)) {
            enum Square from = (us == WHITE) ? e1 : e8;
            enum Square to = (us == WHITE) ? c1 : c8;
            struct Move new_move = create_special_move(CASTLING, PT_NULL, from, to);

            if (legal(new_move, pos, tmp_stack)) {
                *move_list++ = new_move;
                ++num_moves_added;
            }
        }
    }

    stk_destroy(tmp_stack);

    return num_moves_added;
}

int generate_moves(struct Move *move_list, struct Position *pos) {
    int num_moves_added = 0;
    int num_new_moves = 0;

    for (int piece_t = KNIGHT; piece_t <= KING; ++piece_t) {
        num_new_moves = generate_moves_pt(piece_t, move_list, pos);

        num_moves_added += num_new_moves;
        move_list += num_new_moves;
    }

    num_new_moves = generate_pawn_moves(move_list, pos);
    num_moves_added += num_new_moves;

    return num_moves_added;
}