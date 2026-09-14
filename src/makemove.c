#include "peace0x/makemove.h"
#include "peace0x/board.h"
#include "peace0x/move.h"
#include "peace0x/transposition.h"
#include "peace0x/attack.h"

static const int castle_perm_mask[SQUARE_COUNT] = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 13, 15, 15, 15, 12, 15, 15, 14, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15,  7, 15, 15, 15,  3, 15, 15, 11, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15
};

static void clear_piece(Square sq, Board *pos) {
    ASSERT(square_on_board(sq));

    Piece piece = pos->pieces[sq];
    ASSERT(piece_valid(piece));

    Color color = piece_color[piece];
    int piece_num = -1;

    pos->hash_key ^= piece_keys[piece][sq];
    pos->pieces[sq] = PIECE_EMPTY;
    pos->material[color] -= piece_value[piece];

    if (piece_big[piece])   pos->big_pieces[color]--;
    if (piece_minor[piece]) pos->minor_pieces[color]--;
    if (piece_major[piece]) pos->major_pieces[color]--;

    if (piece_pawn[piece]) {
        pos->pawns[color] &= clear_mask[sq64(sq)];
        pos->pawns[COLOR_BOTH] &= clear_mask[sq64(sq)];
    }

    for (int i = 0; i < pos->piece_count[piece]; i++) {
        if (pos->piece_list[piece][i] == sq) {
            piece_num = i;
            break;
        }
    }

    ASSERT(piece_num != -1);
    pos->piece_count[piece]--;
    pos->piece_list[piece][piece_num] = pos->piece_list[piece][pos->piece_count[piece]];
}

static void add_piece(Square sq, Board *pos, Piece piece) {
    ASSERT(square_on_board(sq));
    ASSERT(piece_valid(piece));

    Color color = piece_color[piece];

    pos->hash_key ^= piece_keys[piece][sq];
    pos->pieces[sq] = piece;

    if (piece_big[piece])   pos->big_pieces[color]++;
    if (piece_minor[piece]) pos->minor_pieces[color]++;
    if (piece_major[piece]) pos->major_pieces[color]++;

    pos->material[color] += piece_value[piece];

    if (piece_pawn[piece]) {
        pos->pawns[color] |= set_mask[sq64(sq)];
        pos->pawns[COLOR_BOTH] |= set_mask[sq64(sq)];
    }

    pos->piece_list[piece][pos->piece_count[piece]++] = sq;
}

static void move_piece(Square from, Square to, Board *pos) {
    ASSERT(square_on_board(from));
    ASSERT(square_on_board(to));

    Piece piece = pos->pieces[from];
    Color color = piece_color[piece];

    pos->hash_key ^= piece_keys[piece][from];
    pos->pieces[from] = PIECE_EMPTY;

    pos->hash_key ^= piece_keys[piece][to];
    pos->pieces[to] = piece;

    if (piece_pawn[piece]) {
        pos->pawns[color] &= clear_mask[sq64(from)];
        pos->pawns[COLOR_BOTH] &= clear_mask[sq64(from)];
        pos->pawns[color] |= set_mask[sq64(to)];
        pos->pawns[COLOR_BOTH] |= set_mask[sq64(to)];
    }

    for (int i = 0; i < pos->piece_count[piece]; i++) {
        if (pos->piece_list[piece][i] == from) {
            pos->piece_list[piece][i] = to;
            break;
        }
    }
}

bool make_move(Board *pos, Move move) {
    ASSERT(board_check(pos));

    Square from     = move_from(move);
    Square to       = move_to(move);
    Color  side     = pos->side;
    Piece  captured = move_captured(move);

    ASSERT(square_on_board(from));
    ASSERT(square_on_board(to));
    ASSERT(side_valid(side));
    ASSERT(piece_valid(pos->pieces[from]));

    pos->history[pos->history_ply].hash_key    = pos->hash_key;
    pos->history[pos->history_ply].move        = move;
    pos->history[pos->history_ply].fifty_move  = pos->fifty_move;
    pos->history[pos->history_ply].en_passant  = pos->en_passant;
    pos->history[pos->history_ply].castle_perm = pos->castle_perm;

    if (pos->en_passant != SQ_NONE) {
        pos->hash_key ^= piece_keys[PIECE_EMPTY][pos->en_passant];
    }
    pos->hash_key ^= castle_key[pos->castle_perm];

    if (move_is_en_passant(move)) {
        if (side == COLOR_WHITE) {
            clear_piece((Square)(to - 10), pos);
        } else {
            clear_piece((Square)(to + 10), pos);
        }
    } else if (move_is_castled(move)) {
        switch (to) {
            case SQ_C1: move_piece(SQ_A1, SQ_D1, pos); break;
            case SQ_C8: move_piece(SQ_A8, SQ_D8, pos); break;
            case SQ_G1: move_piece(SQ_H1, SQ_F1, pos); break;
            case SQ_G8: move_piece(SQ_H8, SQ_F8, pos); break;
            default: ASSERT(false); break;
        }
    }

    pos->castle_perm &= castle_perm_mask[from];
    pos->castle_perm &= castle_perm_mask[to];
    pos->en_passant = SQ_NONE;

    pos->hash_key ^= castle_key[pos->castle_perm];

    pos->fifty_move++;

    if (captured != PIECE_EMPTY) {
        ASSERT(piece_valid(captured));
        clear_piece(to, pos);
        pos->fifty_move = 0;
    }

    pos->history_ply++;
    pos->ply++;

    if (piece_pawn[pos->pieces[from]]) {
        pos->fifty_move = 0;
        if (move_is_pawn_start(move)) {
            if (side == COLOR_WHITE) {
                pos->en_passant = (Square)(from + 10);
                ASSERT(ranks_board[pos->en_passant] == RANK_3);
            } else {
                pos->en_passant = (Square)(from - 10);
                ASSERT(ranks_board[pos->en_passant] == RANK_6);
            }
            pos->hash_key ^= piece_keys[PIECE_EMPTY][pos->en_passant];
        }
    }

    move_piece(from, to, pos);

    Piece promoted = move_promoted(move);
    if (promoted != PIECE_EMPTY) {
        ASSERT(piece_valid(promoted) && !piece_pawn[promoted]);
        clear_piece(to, pos);
        add_piece(to, pos, promoted);
    }

    if (piece_king[pos->pieces[to]]) {
        pos->king_square[side] = to;
    }

    pos->side ^= 1;
    pos->hash_key ^= side_key;

    ASSERT(board_check(pos));

    if (is_square_attacked(pos->king_square[side], pos->side, pos)) {
        take_move(pos);
        return false;
    }

    return true;
}

void take_move(Board *pos) {
    ASSERT(board_check(pos));

    pos->history_ply--;
    pos->ply--;

    Move move = pos->history[pos->history_ply].move;
    Square from = move_from(move);
    Square to   = move_to(move);

    ASSERT(square_on_board(from));
    ASSERT(square_on_board(to));

    if (pos->en_passant != SQ_NONE) {
        pos->hash_key ^= piece_keys[PIECE_EMPTY][pos->en_passant];
    }
    pos->hash_key ^= castle_key[pos->castle_perm];

    pos->castle_perm = pos->history[pos->history_ply].castle_perm;
    pos->fifty_move  = pos->history[pos->history_ply].fifty_move;
    pos->en_passant  = pos->history[pos->history_ply].en_passant;

    if (pos->en_passant != SQ_NONE) {
        pos->hash_key ^= piece_keys[PIECE_EMPTY][pos->en_passant];
    }
    pos->hash_key ^= castle_key[pos->castle_perm];

    pos->side ^= 1;
    pos->hash_key ^= side_key;

    if (move_is_en_passant(move)) {
        if (pos->side == COLOR_WHITE) {
            add_piece((Square)(to - 10), pos, PIECE_B_PAWN);
        } else {
            add_piece((Square)(to + 10), pos, PIECE_W_PAWN);
        }
    } else if (move_is_castled(move)) {
        switch (to) {
            case SQ_C1: move_piece(SQ_D1, SQ_A1, pos); break;
            case SQ_C8: move_piece(SQ_D8, SQ_A8, pos); break;
            case SQ_G1: move_piece(SQ_F1, SQ_H1, pos); break;
            case SQ_G8: move_piece(SQ_F8, SQ_H8, pos); break;
            default: ASSERT(false); break;
        }
    }

    move_piece(to, from, pos);

    if (piece_king[pos->pieces[from]]) {
        pos->king_square[pos->side] = from;
    }

    Piece captured = move_captured(move);
    if (captured != PIECE_EMPTY) {
        ASSERT(piece_valid(captured));
        add_piece(to, pos, captured);
    }

    Piece promoted = move_promoted(move);
    if (promoted != PIECE_EMPTY) {
        ASSERT(piece_valid(promoted) && !piece_pawn[promoted]);
        clear_piece(from, pos);
        add_piece(from, pos, (piece_color[promoted] == COLOR_WHITE ? PIECE_W_PAWN : PIECE_B_PAWN));
    }

    ASSERT(board_check(pos));
}

void make_null_move(Board *pos) {
    ASSERT(board_check(pos));
    ASSERT(!is_square_attacked(pos->king_square[pos->side], pos->side ^ 1, pos));

    pos->ply++;
    pos->history[pos->history_ply].hash_key    = pos->hash_key;
    pos->history[pos->history_ply].move        = MOVE_NONE;
    pos->history[pos->history_ply].fifty_move  = pos->fifty_move;
    pos->history[pos->history_ply].en_passant  = pos->en_passant;
    pos->history[pos->history_ply].castle_perm = pos->castle_perm;

    if (pos->en_passant != SQ_NONE) {
        pos->hash_key ^= piece_keys[PIECE_EMPTY][pos->en_passant];
    }
    pos->en_passant = SQ_NONE;

    pos->side ^= 1;
    pos->history_ply++;
    pos->hash_key ^= side_key;

    ASSERT(board_check(pos));
}

void take_null_move(Board *pos) {
    ASSERT(board_check(pos));

    pos->history_ply--;
    pos->ply--;

    if (pos->en_passant != SQ_NONE) {
        pos->hash_key ^= piece_keys[PIECE_EMPTY][pos->en_passant];
    }

    pos->castle_perm = pos->history[pos->history_ply].castle_perm;
    pos->fifty_move  = pos->history[pos->history_ply].fifty_move;
    pos->en_passant  = pos->history[pos->history_ply].en_passant;

    if (pos->en_passant != SQ_NONE) {
        pos->hash_key ^= piece_keys[PIECE_EMPTY][pos->en_passant];
    }

    pos->side ^= 1;
    pos->hash_key ^= side_key;

    ASSERT(board_check(pos));
}
