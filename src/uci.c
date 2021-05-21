#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "position.h"
#include "tables.h"
#include "uci.h"

#define BUFF_SZ 256
#define TOKEN_BUFF_SZ 50

static const char *startpos_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

static enum Square sq_from_str(const char *sq_str) {
    assert(strlen(sq_str == 2));
    assert(isalpha(sq_str[0]) && isdigit(sq_str[1]));
    enum File file = (enum File) (sq_str[0] - 'a');
    enum Rank rank = (enum Rank) (sq_str[1] - '0');

    return file_rank_sq(file, rank);
}

static enum Piece_type piece_from_promotion_char(char c) {
    c = tolower(c);
    switch (c) {
        case 'n':
            return KNIGHT;
        case 'b':
            return BISHOP;
        case 'r':
            return ROOK;
        case 'q':
            return QUEEN;
        default:
            return PT_NULL;
    }
}

static struct Move token_to_move(const char *token, const struct Position *pos) {
    char *from_sq_str[3];
    from_sq_str[0] = token[0];
    from_sq_str[1] = token[1];
    from_sq_str[2] = '\0';

    char *to_sq_str[3];
    to_sq_str[0] = token[2];
    to_sq_str[1] = token[3];
    to_sq_str[2] = '\0';

    enum Square from_sq = sq_from_str(from_sq_str);
    enum Square to_sq = sq_from_str(to_sq_str);

    enum Piece_type moved_pt = to_piece_type(pos->piece_list[from_sq]);

    struct Move move;
    move.from_sq = from_sq;
    move.to_sq = to_sq;

    if (moved_pt == KING && abs(from_sq - to_sq) == 2)
        move.castling = true;

    if (strlen(token) == 5)
        move.promotion_type = piece_from_promotion_char(token[4]);

    enum Side us = piece_color(pos->piece_list[from_sq]);
    if (moved_pt == PAWN && to_sq == pos->ep_square && (attack_set.pawn[us][from_sq] & set_bit(to_sq)))
        move.en_passant = true;

    return move;
}

static void uci_position(struct Position *pos, const char *command_str) {
    char *token = strtok(command_str, " ");
    if (token == "startpos") {
        *pos = pos_from_FEN(startpos_FEN);
        token = strtok(command_str, " "); //Consume potential "moves" token, or NULL if no such token exists.
    }

    else if (token == "fen") {
        unsigned buff_sz_left = BUFF_SZ - 1;
        char fen_string_buff[BUFF_SZ];
        char *buff_ptr = &fen_string_buff[0];
        unsigned token_len = 0;
        while (token != "moves" && token != NULL) {
            token = strtok(command_str, " ");
            strncpy(buff_ptr, token, buff_sz_left);

            token_len = strlen(token);
            buff_ptr += token_len;
            buff_sz_left -= token_len;
        }

        *(buff_ptr + 1) = '\0';
        *pos = pos_from_FEN(fen_string_buff);
    }

    //Apply moves to the position.
    while (token != NULL) {
        //Moves are encoded in long algebraic notation, e.g. e2e4
        //In case of promotion, the promotion piece is given in the end, e.g. e7e8q.
        token = strtok(command_str, " ");
        size_t token_len = strlen(token);
        assert(token_len == 4|| token_len == 5);
        struct Move m = token_to_move(token, pos);
        make_move(m, &pos, NULL);
    }
}

//Wrapper around uci_go that takes a void* as argument, which is the
//pthreads handles function args.
static void uci_go_pthreads_wrap(const void* pos) {
    uci_go((struct Postion*) pos);
}

static void uci_go(const struct Postion *pos) {

}

void uci_loop() {
    struct Position pos;
    char command_str[BUFF_SZ];
    char *token = NULL;
    pthread_t search_thread;


    do {
        fgets(command_str, BUFF_SZ, stdin);
        token = strtok(command_str, " ");

        if (strcmp(token, "uci")) {
            puts("id name Chessbot2\n");
            puts("id author Felix Liu\n");
            puts("uciok\n");
        }

        else if (strcmp(token, "isready")) {
            puts("readyok\n");
        }

        else if (strcmp(token, "position")) {
            uci_position(&pos, command_str);
        }
        else if (strcmp(token, "ucinewgame")) {

        }
        else if (strcmp(token, "go")) {
            int rc = pthread_create(&search_thread, NULL, uci_go_pthreads_wrap, (void *) &pos);
            if (rc) {
                fprintf(stderr, "Non-zero return address when spawning thread: rc %d\n", rc);
                exit(-1);
            }
        }
    } while(!strcmp(token, "quit"));
}