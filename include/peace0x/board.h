#ifndef PEACE0X_BOARD_H
#define PEACE0X_BOARD_H

#include "peace0x/types.h"
#include "peace0x/bitboard.h"

/* Coordinate lookup tables */
extern int    sq120_to_sq64[SQUARE_COUNT];
extern Square sq64_to_sq120[BOARD_SQUARE_COUNT];
extern int    files_board[SQUARE_COUNT];
extern int    ranks_board[SQUARE_COUNT];
extern int    mirror64[BOARD_SQUARE_COUNT];

/* Piece characteristics */
extern const char  piece_char[];
extern const char  side_char[];
extern const char  rank_char[];
extern const char  file_char[];

extern const bool  piece_big[PIECE_COUNT];
extern const bool  piece_major[PIECE_COUNT];
extern const bool  piece_minor[PIECE_COUNT];
extern const int   piece_value[PIECE_COUNT];
extern const Color piece_color[PIECE_COUNT];
extern const bool  piece_pawn[PIECE_COUNT];
extern const bool  piece_knight[PIECE_COUNT];
extern const bool  piece_king[PIECE_COUNT];
extern const bool  piece_rook_queen[PIECE_COUNT];
extern const bool  piece_bishop_queen[PIECE_COUNT];
extern const bool  piece_slides[PIECE_COUNT];

/* Inline coordinate helpers */
static inline Square sq120(int s64) {
    return sq64_to_sq120[s64];
}

static inline int sq64(Square s120) {
    return sq120_to_sq64[s120];
}

static inline Square file_rank_to_sq(File f, Rank r) {
    return (Square)((21 + (int)f) + ((int)r * 10));
}

static inline bool square_on_board(Square sq) {
    return sq >= 0 && sq < SQUARE_COUNT && files_board[sq] != (int)SQ_OFFBOARD;
}

static inline bool side_valid(Color side) {
    return side == COLOR_WHITE || side == COLOR_BLACK;
}

static inline bool file_rank_valid(int fr) {
    return fr >= 0 && fr <= 7;
}

static inline bool piece_valid(Piece p) {
    return p >= PIECE_W_PAWN && p <= PIECE_B_KING;
}

static inline bool piece_valid_empty(Piece p) {
    return p >= PIECE_EMPTY && p <= PIECE_B_KING;
}

static inline int mirror_64(int s64) {
    return mirror64[s64];
}

/* Board lifecycle and state functions */
void board_init(Board *pos);
void board_reset(Board *pos);
int  board_parse_fen(const char *fen, Board *pos);
void board_print(const Board *pos);
void board_mirror(Board *pos);
bool board_check(const Board *pos);
void board_update_lists_material(Board *pos);

#endif /* PEACE0X_BOARD_H */
