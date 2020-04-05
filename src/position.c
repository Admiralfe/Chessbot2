#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "bitboard.h"
#include "position.h"
#include "types.h"

//LUT used to get square name from square index. The square index is ordered according
//to the Square enum.
const char *square_name_LUT[] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};

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

struct Move create_special_move(enum Move_type type, enum Piece_type promotion_type, enum Square from, enum Square to) {
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
            m.promotion_type = promotion_type;
            break;
        case NORMAL:
            break;
    }

    return m;
}

/*
    Takes a pseudo-legal move and checks for legality.
    In practice, this mostly involves checking if the move leaves the king
    in check and some other special cases.
*/
bool legal(struct Move m, const struct Position *pos) {
    //Is the king attacked after the move?
}

static void store_move_state(const struct Position *pos, struct Move m, MS_Stack *move_state_stack) {
    //Store the irreversible state of this position to be able to unmake moves
    struct Move_state ms = {0};
    ms.half_move_clock = pos->half_move_clock;
    ms.can_castle[WHITE] = pos->can_kingside_castle[WHITE] | pos->can_queenside_castle[WHITE];
    ms.can_castle[BLACK] = pos->can_kingside_castle[BLACK] | pos->can_queenside_castle[BLACK];
    ms.caputured_piece = pos->piece_list[m.to_sq];
    ms.ep_square = pos->ep_square;
    stk_push(move_state_stack, &ms);
}

void make_move(struct Move m, struct Position *pos, MS_Stack *move_state_stk) {
    const enum Side us = pos->side_to_move;
    const enum Piece moved_piece = pos->piece_list[m.from_sq];
    const enum Piece_type moved_piece_type = to_piece_type(moved_piece);

    store_move_state(pos, m, move_state_stk);

    //If the king moves, the right to castle is lost.
    if (moved_piece_type == KING) {
        pos->can_kingside_castle[us] = false;
        pos->can_queenside_castle[us] = false;
    }

    //If the move is a pawn move or a capture, the half-move clock (for the 50 move rule) is reset
    if ((pos->piece_list[m.to_sq] != PIECE_EMPTY) || moved_piece_type == PAWN)
        pos->half_move_clock = 0;
    else //Otherwise, the half_move_clock is incremented by 1
        ++pos->half_move_clock;

    //All possible types of moves leave the from square empty.
    pos->piece_list[m.from_sq] = PIECE_EMPTY;
    if (m.castling) {
        assert((set_bit(m.to_sq) & FileGBB) | (set_bit(m.to_sq) & FileBBB));
        pos->piece_list[m.to_sq] = to_colored_piece(KING, us);
        //Short castling
        if (set_bit(m.to_sq) & FileGBB)
            //Rook is always one to the right of king in short castling
            pos->piece_list[m.to_sq - 1] = to_colored_piece(ROOK, us);
        //Long castling
        else 
            //Rook is always one to the left of king in long castling
            pos->piece_list[m.to_sq + 1] = to_colored_piece(ROOK, us);
    }

    else if (m.en_passant) {
        pos->piece_list[m.to_sq] = to_colored_piece(PAWN, us);
        //The captured pawn's square is either above or below the destination square, depending on side.
        const int pawn_sq = (us == WHITE) ? m.to_sq - 8 : m.to_sq + 8;
        pos->piece_list[pawn_sq] = PIECE_EMPTY;
    }

    else
        pos->piece_list[m.to_sq] = moved_piece;

    //Change the side to move
    pos->side_to_move = abs(us - 1);

    if (us == BLACK)
        ++pos->fullmove_count;

    pos_from_piece_list(pos);
}

void unmake_move(struct Move m, struct Position *pos, MS_Stack *move_state_stk) {
    const enum Piece moved_piece = pos->piece_list[m.to_sq];
    const struct Move_state prev_move_state = stk_pop(move_state_stk); // Move state with irreversible aspects of previous position
    const enum Side moving_side = abs(pos->side_to_move - 1); // Side that made the move we are undoing now
    const enum Side other_side = pos->side_to_move;

    //Restore potential captured piece, or empty square
    pos->piece_list[m.to_sq] = prev_move_state.caputured_piece;

    if (m.castling) {
        pos->piece_list[m.from_sq] = to_colored_piece(KING, moving_side);
        //Kingside castling
        if (m.to_sq & FileGBB)
            //3 squares to the right of the king square is the rook in short castling
            pos->piece_list[m.from_sq + 3] = to_colored_piece(ROOK, moving_side);
        //Queenside castling
        else
            //4 squares to the left of the king is the rook in long castling
            pos->piece_list[m.from_sq - 4] = to_colored_piece(ROOK, moving_side);
    }

    else if (m.en_passant) {
        pos->piece_list[m.from_sq] = to_colored_piece(PAWN, moving_side);
        const int offset = (moving_side == WHITE) ? -8 : 8; 
        //In en passant moves, one pawn will have been captured above the to_square
        pos->piece_list[m.to_sq + offset] = to_colored_piece(PAWN, other_side);
    }
    else
        pos->piece_list[m.from_sq] = moved_piece;

    pos->half_move_clock = prev_move_state.half_move_clock;
    pos->ep_square = prev_move_state.ep_square;
    pos->can_kingside_castle[WHITE] = prev_move_state.can_castle[WHITE];
    pos->can_queenside_castle[WHITE] = prev_move_state.can_castle[WHITE];
    pos->can_kingside_castle[BLACK] = prev_move_state.can_castle[BLACK];
    pos->can_queenside_castle[BLACK] = prev_move_state.can_castle[BLACK];
    pos->side_to_move = abs(pos->side_to_move - 1);
    
    pos_from_piece_list(pos);
}

void init_pos_struct(struct Position *pos) {
    for (int piece_type = PAWN; piece_type <= KING; ++piece_type) {
        for (int side = 0; side < 2; ++side)
            pos->piece_bb[piece_type][side] = 0ULL;
    }

    pos->occupied_squares[WHITE] = 0ULL;
    pos->occupied_squares[BLACK] = 0ULL;

    pos->empty_squares = 0ULL;
}

void pos_from_piece_list(struct Position *pos) {
    for (int i = 0; i < 64; ++i) {
        if (pos->piece_list[i] != PIECE_EMPTY) {
            enum Piece_type piece_type = to_piece_type(pos->piece_list[i]);
            enum Side color = piece_color(pos->piece_list[i]);
            pos->piece_bb[piece_type][color] |= set_bit(i);
            pos->occupied_squares[color] |= set_bit(i);
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

    struct Position pos = {}; //C implicitly initializes struct elements to unsigned 0, which is what we want.
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
    pos.can_queenside_castle[WHITE] = pos.can_queenside_castle[BLACK] = false;

    while (!isspace(fen_str[str_idx])) {
        switch (fen_str[str_idx]) {
            case '-':
                break;
            case 'K':
                pos.can_kingside_castle[WHITE] = true;
                break;
            case 'Q':
                pos.can_queenside_castle[WHITE] = true;
                break;
            case 'k':
                pos.can_kingside_castle[BLACK] = true;
                break;
            case 'q':
                pos.can_queenside_castle[BLACK] = true;
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

        else if (set_bit(move.to_sq) & FileCBB)
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

static bool queenside_castling_impeded(enum Side side, const struct Position *pos) {
    return (side == WHITE) ? pos_occupancy(*pos) & (set_bit(b1) | set_bit(c1) | set_bit(d1)) :
                             pos_occupancy(*pos) & (set_bit(b8) | set_bit(c8) | set_bit(d8));
}

static bool kingside_castling_impeded(enum Side side, const struct Position *pos) {
    return (side == WHITE) ? pos_occupancy(*pos) & (set_bit(f1) | set_bit(g1)) : 
                             pos_occupancy(*pos) & (set_bit(f8) | set_bit(g8));
}

bool can_kingside_castle(enum Side side, const struct Position *pos) {
    u64 king_square = (side == WHITE) ? set_bit(e1) : set_bit(e8);
    u64 rook_square = (side == WHITE) ? set_bit(h1) : set_bit(h8);
    return pos->can_kingside_castle[side]
        && (pos->piece_bb[KING][side] & king_square)
        && (pos->piece_bb[ROOK][side] & rook_square)
        && !kingside_castling_impeded(side, pos);
}

bool can_queenside_castle(enum Side side, const struct Position *pos) {
    u64 king_square = (side == WHITE) ? set_bit(e1) : set_bit(e8);
    u64 rook_square = (side == WHITE) ? set_bit(a1) : set_bit(a8);
    return    pos->can_queenside_castle[side]
           && (pos->piece_bb[ROOK][side] & rook_square)
           && (pos->piece_bb[KING][side] & king_square)
           && !queenside_castling_impeded(side, pos);
}