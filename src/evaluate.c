#include "peace0x/evaluate.h"
#include "peace0x/board.h"
#include "peace0x/bitboard.h"
#include <stdlib.h>

const int pawn_isolated       = -10;
const int pawn_passed[8]      = { 0, 5, 10, 20, 35, 60, 100, 200 };
const int rook_open_file      = 10;
const int rook_semi_open_file = 5;
const int queen_open_file     = 5;
const int queen_semi_open_file= 3;
const int bishop_pair         = 30;

const int pawn_table[BOARD_SQUARE_COUNT] = {
     0,  0,  0,   0,   0,  0,  0,  0,
    10, 10,  0, -10, -10,  0, 10, 10,
     5,  0,  0,   5,   5,  0,  0,  5,
     0,  0, 10,  20,  20, 10,  0,  0,
     5,  5,  5,  10,  10,  5,  5,  5,
    10, 10, 10,  20,  20, 10, 10, 10,
    20, 20, 20,  30,  30, 20, 20, 20,
     0,  0,  0,   0,   0,  0,  0,  0
};

const int knight_table[BOARD_SQUARE_COUNT] = {
     0, -10,  0,  0,  0,  0, -10,  0,
     0,   0,  0,  5,  5,  0,   0,  0,
     0,   0, 10, 10, 10, 10,   0,  0,
     0,   0, 10, 20, 20, 10,   5,  0,
     5,  10, 15, 20, 20, 15,  10,  5,
     5,  10, 10, 20, 20, 10,  10,  5,
     0,   0,  5, 10, 10,  5,   0,  0,
     0,   0,  0,  0,  0,  0,   0,  0
};

const int bishop_table[BOARD_SQUARE_COUNT] = {
     0,  0, -10,  0,  0, -10,  0,  0,
     0,  0,   0, 10, 10,   0,  0,  0,
     0,  0,  10, 15, 15,  10,  0,  0,
     0, 10,  15, 20, 20,  15, 10,  0,
     0, 10,  15, 20, 20,  15, 10,  0,
     0,  0,  10, 15, 15,  10,  0,  0,
     0,  0,   0, 10, 10,   0,  0,  0,
     0,  0,   0,  0,  0,   0,  0,  0
};

const int rook_table[BOARD_SQUARE_COUNT] = {
     0,  0,  5, 10, 10,  5,  0,  0,
     0,  0,  5, 10, 10,  5,  0,  0,
     0,  0,  5, 10, 10,  5,  0,  0,
     0,  0,  5, 10, 10,  5,  0,  0,
     0,  0,  5, 10, 10,  5,  0,  0,
     0,  0,  5, 10, 10,  5,  0,  0,
    25, 25, 25, 25, 25, 25, 25, 25,
     0,  0,  5, 10, 10,  5,  0,  0
};

const int king_endgame[BOARD_SQUARE_COUNT] = {
    -50, -10,  0,  0,  0,  0, -10, -50,
    -10,   0, 10, 10, 10, 10,   0, -10,
      0,  10, 15, 15, 15, 15,  10,   0,
      0,  10, 15, 20, 20, 15,  10,   0,
      0,  10, 15, 20, 20, 15,  10,   0,
      0,  10, 15, 15, 15, 15,  10,   0,
    -10,   0, 10, 10, 10, 10,   0, -10,
    -50, -10,  0,  0,  0,  0, -10, -50
};

const int king_opening[BOARD_SQUARE_COUNT] = {
      0,   5,   5, -10, -10,   0,  10,   5,
    -30, -30, -30, -30, -30, -30, -30, -30,
    -50, -50, -50, -50, -50, -50, -50, -50,
    -70, -70, -70, -70, -70, -70, -70, -70,
    -70, -70, -70, -70, -70, -70, -70, -70,
    -70, -70, -70, -70, -70, -70, -70, -70,
    -70, -70, -70, -70, -70, -70, -70, -70,
    -70, -70, -70, -70, -70, -70, -70, -70
};

#define ENDGAME_MATERIAL (1 * piece_value[PIECE_W_ROOK] + 2 * piece_value[PIECE_W_KNIGHT] + 2 * piece_value[PIECE_W_PAWN] + piece_value[PIECE_W_KING])

bool is_material_draw(const Board *pos) {
    if (!pos->piece_count[PIECE_W_ROOK] && !pos->piece_count[PIECE_B_ROOK] &&
        !pos->piece_count[PIECE_W_QUEEN] && !pos->piece_count[PIECE_B_QUEEN]) {

        if (!pos->piece_count[PIECE_B_BISHOP] && !pos->piece_count[PIECE_W_BISHOP]) {
            if (pos->piece_count[PIECE_W_KNIGHT] < 3 && pos->piece_count[PIECE_B_KNIGHT] < 3) {
                return true;
            } else if (!pos->piece_count[PIECE_W_KNIGHT] && !pos->piece_count[PIECE_B_KNIGHT]) {
                if (abs(pos->piece_count[PIECE_W_BISHOP] - pos->piece_count[PIECE_B_BISHOP]) < 2) {
                    return true;
                }
            } else if ((pos->piece_count[PIECE_W_KNIGHT] < 3 && !pos->piece_count[PIECE_W_BISHOP]) ||
                       (pos->piece_count[PIECE_W_BISHOP] == 1 && !pos->piece_count[PIECE_W_KNIGHT])) {
                if ((pos->piece_count[PIECE_B_KNIGHT] < 3 && !pos->piece_count[PIECE_B_BISHOP]) ||
                    (pos->piece_count[PIECE_B_BISHOP] == 1 && !pos->piece_count[PIECE_B_KNIGHT])) {
                    return true;
                }
            }
        }
    } else if (!pos->piece_count[PIECE_W_QUEEN] && !pos->piece_count[PIECE_B_QUEEN]) {
        if (pos->piece_count[PIECE_W_ROOK] == 1 && pos->piece_count[PIECE_B_ROOK] == 1) {
            if ((pos->piece_count[PIECE_W_KNIGHT] + pos->piece_count[PIECE_W_BISHOP]) < 2 &&
                (pos->piece_count[PIECE_B_KNIGHT] + pos->piece_count[PIECE_B_BISHOP]) < 2) {
                return true;
            }
        } else if (pos->piece_count[PIECE_W_ROOK] == 1 && pos->piece_count[PIECE_B_ROOK] == 0) {
            if ((pos->piece_count[PIECE_W_KNIGHT] + pos->piece_count[PIECE_W_BISHOP] == 0) &&
                (((pos->piece_count[PIECE_B_KNIGHT] + pos->piece_count[PIECE_B_BISHOP]) == 1) ||
                 ((pos->piece_count[PIECE_B_KNIGHT] + pos->piece_count[PIECE_B_BISHOP]) == 2))) {
                return true;
            }
        } else if (pos->piece_count[PIECE_B_ROOK] == 1 && pos->piece_count[PIECE_W_ROOK] == 0) {
            if ((pos->piece_count[PIECE_B_KNIGHT] + pos->piece_count[PIECE_B_BISHOP] == 0) &&
                (((pos->piece_count[PIECE_W_KNIGHT] + pos->piece_count[PIECE_W_BISHOP]) == 1) ||
                 ((pos->piece_count[PIECE_W_KNIGHT] + pos->piece_count[PIECE_W_BISHOP]) == 2))) {
                return true;
            }
        }
    }
    return false;
}

Score evaluate_position(const Board *pos) {
    ASSERT(board_check(pos));

    Score score = pos->material[COLOR_WHITE] - pos->material[COLOR_BLACK];

    if (!pos->piece_count[PIECE_W_PAWN] && !pos->piece_count[PIECE_B_PAWN] && is_material_draw(pos)) {
        return 0;
    }

    /* White Pawns */
    Piece piece = PIECE_W_PAWN;
    for (int i = 0; i < pos->piece_count[piece]; i++) {
        Square sq = pos->piece_list[piece][i];
        ASSERT(square_on_board(sq));
        int s64 = sq64(sq);
        score += pawn_table[s64];

        if ((isolated_pawn_mask[s64] & pos->pawns[COLOR_WHITE]) == 0ULL) {
            score += pawn_isolated;
        }

        if ((white_passed_mask[s64] & pos->pawns[COLOR_BLACK]) == 0ULL) {
            score += pawn_passed[ranks_board[sq]];
        }
    }

    /* Black Pawns */
    piece = PIECE_B_PAWN;
    for (int i = 0; i < pos->piece_count[piece]; i++) {
        Square sq = pos->piece_list[piece][i];
        ASSERT(square_on_board(sq));
        int s64 = sq64(sq);
        score -= pawn_table[mirror_64(s64)];

        if ((isolated_pawn_mask[s64] & pos->pawns[COLOR_BLACK]) == 0ULL) {
            score -= pawn_isolated;
        }

        if ((black_passed_mask[s64] & pos->pawns[COLOR_WHITE]) == 0ULL) {
            score -= pawn_passed[7 - ranks_board[sq]];
        }
    }

    /* White Knights */
    piece = PIECE_W_KNIGHT;
    for (int i = 0; i < pos->piece_count[piece]; i++) {
        Square sq = pos->piece_list[piece][i];
        ASSERT(square_on_board(sq));
        score += knight_table[sq64(sq)];
    }

    /* Black Knights */
    piece = PIECE_B_KNIGHT;
    for (int i = 0; i < pos->piece_count[piece]; i++) {
        Square sq = pos->piece_list[piece][i];
        ASSERT(square_on_board(sq));
        score -= knight_table[mirror_64(sq64(sq))];
    }

    /* White Bishops */
    piece = PIECE_W_BISHOP;
    for (int i = 0; i < pos->piece_count[piece]; i++) {
        Square sq = pos->piece_list[piece][i];
        ASSERT(square_on_board(sq));
        score += bishop_table[sq64(sq)];
    }

    /* Black Bishops */
    piece = PIECE_B_BISHOP;
    for (int i = 0; i < pos->piece_count[piece]; i++) {
        Square sq = pos->piece_list[piece][i];
        ASSERT(square_on_board(sq));
        score -= bishop_table[mirror_64(sq64(sq))];
    }

    /* White Rooks */
    piece = PIECE_W_ROOK;
    for (int i = 0; i < pos->piece_count[piece]; i++) {
        Square sq = pos->piece_list[piece][i];
        ASSERT(square_on_board(sq));
        score += rook_table[sq64(sq)];

        int file = files_board[sq];
        if (!(pos->pawns[COLOR_BOTH] & file_bitboard_mask[file])) {
            score += rook_open_file;
        } else if (!(pos->pawns[COLOR_WHITE] & file_bitboard_mask[file])) {
            score += rook_semi_open_file;
        }
    }

    /* Black Rooks */
    piece = PIECE_B_ROOK;
    for (int i = 0; i < pos->piece_count[piece]; i++) {
        Square sq = pos->piece_list[piece][i];
        ASSERT(square_on_board(sq));
        score -= rook_table[mirror_64(sq64(sq))];

        int file = files_board[sq];
        if (!(pos->pawns[COLOR_BOTH] & file_bitboard_mask[file])) {
            score -= rook_open_file;
        } else if (!(pos->pawns[COLOR_BLACK] & file_bitboard_mask[file])) {
            score -= rook_semi_open_file;
        }
    }

    /* White Queens */
    piece = PIECE_W_QUEEN;
    for (int i = 0; i < pos->piece_count[piece]; i++) {
        Square sq = pos->piece_list[piece][i];
        ASSERT(square_on_board(sq));

        int file = files_board[sq];
        if (!(pos->pawns[COLOR_BOTH] & file_bitboard_mask[file])) {
            score += queen_open_file;
        } else if (!(pos->pawns[COLOR_WHITE] & file_bitboard_mask[file])) {
            score += queen_semi_open_file;
        }
    }

    /* Black Queens */
    piece = PIECE_B_QUEEN;
    for (int i = 0; i < pos->piece_count[piece]; i++) {
        Square sq = pos->piece_list[piece][i];
        ASSERT(square_on_board(sq));

        int file = files_board[sq];
        if (!(pos->pawns[COLOR_BOTH] & file_bitboard_mask[file])) {
            score -= queen_open_file;
        } else if (!(pos->pawns[COLOR_BLACK] & file_bitboard_mask[file])) {
            score -= queen_semi_open_file;
        }
    }

    /* Kings */
    Square white_king_sq = pos->piece_list[PIECE_W_KING][0];
    if (pos->material[COLOR_BLACK] <= ENDGAME_MATERIAL) {
        score += king_endgame[sq64(white_king_sq)];
    } else {
        score += king_opening[sq64(white_king_sq)];
    }

    Square black_king_sq = pos->piece_list[PIECE_B_KING][0];
    if (pos->material[COLOR_WHITE] <= ENDGAME_MATERIAL) {
        score -= king_endgame[mirror_64(sq64(black_king_sq))];
    } else {
        score -= king_opening[mirror_64(sq64(black_king_sq))];
    }

    if (pos->piece_count[PIECE_W_BISHOP] >= 2) score += bishop_pair;
    if (pos->piece_count[PIECE_B_BISHOP] >= 2) score -= bishop_pair;

    return (pos->side == COLOR_WHITE) ? score : -score;
}
