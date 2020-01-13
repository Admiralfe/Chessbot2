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
            assert(0 && "Invalid char supplied to char_to_piece");
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

struct Move create_regular_move(enum Square from, enum Square to) {
    struct Move m;
    m.from_sq = from;
    m.to_sq = to;
    m.castling = false;
    m.en_passant = false;
    m.promotion_type = PT_NULL;

    return m;
}

struct Move create_special_move(enum Move_type type, enum Piece_type piece_type, enum Square from, enum Square to) {
    struct Move m;
    m.from_sq = from;
    m.to_sq = to;
    m.castling = false;
    m.en_passant = false;
    m.promotion_type = PT_NULL;
    
    switch (type) {
        case CASTLING:
            m.castling = true;
            break;
        case ENPASSANT:
            m.en_passant = true;
            break;
        case PROMOTION:
            m.promotion_type = piece_type;
            break;
        case NORMAL:
            break;
    }

    return m;
}

void init_pos_struct(struct Position *pos) {
    for (int i = 0; i < 2; ++i) {
        pos->pawns[i] = 0ULL;
        pos->knights[i] = 0ULL;
        pos->rooks[i] = 0ULL;
        pos->kings[i] = 0ULL;
        pos->queens[i] = 0ULL;
        pos->occupied_squares[i] = 0ULL;
    }

    pos->empty_squares = 0ULL;
}

void pos_from_piece_list(struct Position *pos) {
    for (int i = 0; i < 64; ++i) {
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
    int board_idx = (int) a8; //FEN strings start from rank 8.
    int str_idx = 0;

    struct Position pos = {.pawns[0] = 0ULL }; //C implicitly initializes struct elements to unsigned 0, which is what we want.
    while (!isspace(fen_str[str_idx])) {
        if (isdigit(fen_str[str_idx])) {
            for (int i = 0; i < (fen_str[str_idx] - '0'); ++i) {
                pos.piece_list[board_idx] = PIECE_EMPTY;
                ++board_idx;
            }
        }

        else if (fen_str[str_idx] != '/'){
            pos.piece_list[board_idx] = char_to_piece(fen_str[str_idx]);
            ++board_idx;
        }

        else { //Means the current character is a '/'
            board_idx -= 16; //When we go to the next rank, we need to wrap the index back around
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
        ++str_idx;
    } else {
        assert(isalpha(fen_str[str_idx]) && isdigit(fen_str[str_idx + 1]) &&
               "Incorrect EP square format in FEN string");
        //The square is given in algebraic notation, e.g. e4.
        //So we take the row index multiplied by 8, and add the column index.
        pos.ep_square = (enum Square) 8 * (fen_str[str_idx + 1] - '1') + (fen_str[str_idx] - 'a');
        str_idx += 2;
    }
    
    assert(isspace(fen_str[str_idx]) &&
           "Incorrectly formatted FEN string, expected space after ep_square");
    ++str_idx;

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
        pos.half_move_clock = atoi( tmp);
    }

    return pos;
}

void print_move(struct Move move, struct Position pos) {
    enum Piece piece_type = pos.piece_list[move.from_sq];

    if (move.castling) {
        //Short castling
        if (set_bit(move.to_sq) & FileGBB)
            printf("O-O\n");

        else if (set_bit(move.to_sq) & FileBBB)
            printf("O-O-O\n");

        return;
    }

    switch (piece_type) {
        case WHITE_PAWN:
        case BLACK_PAWN:
            break;
        case WHITE_KNIGHT:
        case BLACK_KNIGHT:
            printf("N");
            break;
        case WHITE_KING:
        case BLACK_KING:
            printf("K");
            break;
        case WHITE_BISHOP:
        case BLACK_BISHOP:
            printf("B");
            break;
        case WHITE_ROOK:
        case BLACK_ROOK:
            printf("R");
            break;
        case WHITE_QUEEN:
        case BLACK_QUEEN:
            printf("Q");
            break;
        default:
            printf("Invalid move encountered in print move.\n");
            break;
    }

    //If the target square intersects with any of the pieces, or the move is en passant
    //the move is a capture.
    if ((set_bit(move.to_sq) & pos_occupancy(pos)) || move.en_passant) {
        printf("%sx%s", square_name_LUT[move.from_sq], square_name_LUT[move.to_sq]);
    }

    else { //Not a capture
        printf("%s-%s", square_name_LUT[move.from_sq], square_name_LUT[move.to_sq]);
    }

    //Promotions need extra text
    if (move.promotion_type != PT_NULL) {
        switch (move.promotion_type) {
            case KNIGHT:
                printf("=N");
                break;
            case BISHOP:
                printf("=B");
                break;
            case ROOK:
                printf("=R");
                break;
            case QUEEN:
                printf("=Q");
                break;
            default:
                break;
        }
    }

    printf("\n");
}