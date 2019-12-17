#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bitboard.h"
#include "position.h"
#include "types.h"

//Array used to get pieces by index. Indexing this array using the Piece enum gives
//The character for the corresponding piece
const char* piece_to_char = " PNBRQKpnbrqk";

enum Piece char_to_piece(char c) {
    switch(c) {
        case 'P':
            return WHITE_PAWN;
        case 'N':
            return WHITE_KNIGHT;
        case 'B':
            return WHITE_BISHOP;
        case 'R':
            return WHITE_ROOK;
        case 'Q':
            return WHITE_QUEEN;
        case 'K':
            return WHITE_KING;
        case 'p':
            return BLACK_PAWN;
        case 'n':
            return BLACK_KNIGHT;
        case 'b':
            return BLACK_BISHOP;
        case 'r':
            return BLACK_ROOK;
        case 'q':
            return BLACK_QUEEN;
        case 'k':
            return BLACK_KING;
        default:
            assert(0 &&
                   "Invalid char supplied to char_to_piece");
    }
}

enum Square make_square(enum Rank r, enum File f) {
    return (enum Square) r * 8 + f;
}

void print_position(struct Position position) {
    char pos_str[1024];

    int idx = 0;

    idx += snprintf(pos_str, 1024 - idx, "\n +---+---+---+---+---+---+---+---+\n"); 

    for (int rank = RANK_1; rank <= RANK_8; ++rank) {
        for (int file = FILE_A; file <= FILE_H; ++file  ) {
            idx += snprintf(&pos_str[idx], 1024 - idx, 
                            "   %c", piece_to_char[get_piece(position, make_square(rank, file))]);
        }
        idx += snprintf(&pos_str[idx], 1024 - idx, "\n +---+---+---+---+---+---+---+---+\n");
    }

    printf("%s", pos_str);
}

void pos_from_piece_list(struct Position *pos) {
    for (int i = 0; i < 64; i++) {
        switch (pos->piece_list[i]) {
            case WHITE_PAWN:
                pos->pawns[WHITE] |= set_bit(i);
                break;
            case WHITE_KNIGHT:
                pos->knights[WHITE] |=  set_bit(i);
                break;
            case WHITE_BISHOP:
                pos->bishops[WHITE] |= set_bit(i);
                break;
            case WHITE_ROOK:
                pos->rooks[WHITE] |= set_bit(i);
                break;
            case WHITE_QUEEN:
                pos->queens[WHITE] |= set_bit(i);
                break;
            case WHITE_KING:
                pos->kings[WHITE] |= set_bit(i);
                break;
            case BLACK_PAWN:
                pos->pawns[BLACK] |= set_bit(i);
                break;
            case BLACK_KNIGHT:
                pos->knights[BLACK] |=  set_bit(i);
                break;
            case BLACK_BISHOP:
                pos->bishops[BLACK] |= set_bit(i);
                break;
            case BLACK_ROOK:
                pos->rooks[BLACK] |= set_bit(i);
                break;
            case BLACK_QUEEN:
                pos->queens[BLACK] |= set_bit(i);
                break;
            case BLACK_KING:
                pos->kings[BLACK] |= set_bit(i);
                break;
            case PIECE_EMPTY:
                break; //Nothing needs to be done
        }
    }
}

/*
Creates a new position struct based on the FEN string supplied.
Currently this function is not very robust, and the correctness of the FEN string is assumed to 
be the responsibility of say a GUI.
*/
struct Position pos_from_FEN(char *fen_str) {
    int board_idx = 0;
    int str_idx = 0;

    struct Position pos;
    while (!isspace(fen_str[str_idx])) {
        if (isdigit(fen_str[str_idx])) {
            for (int i = 0; i < fen_str[str_idx] - '0'; i++) {
                pos.piece_list[board_idx] = PIECE_EMPTY;
                ++board_idx;
            }
        }
        else if (fen_str[str_idx] != '/'){
            pos.piece_list[board_idx] = char_to_piece(fen_str[str_idx]);
            ++board_idx;
        }

        ++str_idx;
    }

    //Fill the bitboards in the position struct based on the piece list
    pos_from_piece_list(&pos);

    assert(isspace(fen_str[str_idx]) &&
           "Incorrectly formated FEN string found after square parsing.");
    //Next character in the FEN string should be a whitespace, which we can skip
    str_idx++;

    if (fen_str[str_idx] == 'w')
        pos.side_to_move = WHITE;
    else if (fen_str[str_idx] == 'b')
        pos.side_to_move = BLACK;

    assert(isspace(fen_str[str_idx + 1]) &&
           "Incorrectly formatted FEN string found after castling side parsing.");
    //Skip the previous character and the following whitespace.
    str_idx += 2;

    //Set default values for castling to false
    pos.can_kingside_castle[WHITE] = pos.can_kingside_castle[BLACK] = false;
    pos.can_queenside_caslte[WHITE] = pos.can_queenside_caslte[BLACK] = false;

    while (!isspace(fen_str[str_idx])) {
        switch (fen_str[str_idx]) {
            case '-':
                break;
            case 'K':
                pos.can_kingside_castle[WHITE] = true;
                break;
            case 'Q':
                pos.can_queenside_caslte[WHITE] = true;
                break;
            case 'k':
                pos.can_kingside_castle[BLACK] = true;
                break;
            case 'q':
                pos.can_queenside_caslte[BLACK] = true;
                break;
            default:
                assert(0 &&
                       "Incorretly formatted FEN string, castling rights");
        }

        str_idx++;
    }

    assert(isspace(fen_str[str_idx]) &&
           "Incorrectly formatted FEN string found.");
    str_idx++;


    if (fen_str[str_idx] == '-') {
        pos.ep_square = SQUARE_EMPTY;
    } else {
        assert(isalpha(fen_str[str_idx]) && isdigit(fen_str[str_idx + 1]) &&
               "Incorrect EP square format in FEN string");
        //The square is given in algebraic notation, e.g. e4.
        //So we take the row index multiplied by 8, and add the column index.
        pos.ep_square = (enum Square) 8 * (fen_str[str_idx] - 'a') + (fen_str[str_idx + 1] - '0');
    }
    
    assert(isspace(fen_str[str_idx + 1]) &&
           "Incorrectly formatted FEN string, expected space after ep_square");
    str_idx += 2;

    //Next in the string should be a 1 or 2 digit number. It should never be 3 digits,
    //Since whenever the half move counter reaches 50, the game is drawn.
    assert(isdigit(fen_str[str_idx]) && (isdigit(fen_str[str_idx + 1]) || isspace(fen_str[str_idx + 1])));

    //one digit number
    if (isspace(fen_str[str_idx + 1]))
        pos.half_move_clock = fen_str[str_idx] - '0';
    else { //Two digit number
        char tmp[3];
        strncpy(tmp, fen_str + str_idx, 2);
        tmp[2] = '\0';
        pos.half_move_clock = atoi(tmp);
    }

    return pos;
}