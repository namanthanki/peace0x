#ifndef PEACE0X_EVALUATE_H
#define PEACE0X_EVALUATE_H

#include "peace0x/types.h"

extern const int pawn_isolated;
extern const int pawn_passed[8];
extern const int rook_open_file;
extern const int rook_semi_open_file;
extern const int queen_open_file;
extern const int queen_semi_open_file;
extern const int bishop_pair;

extern const int pawn_table[BOARD_SQUARE_COUNT];
extern const int knight_table[BOARD_SQUARE_COUNT];
extern const int bishop_table[BOARD_SQUARE_COUNT];
extern const int rook_table[BOARD_SQUARE_COUNT];
extern const int king_opening[BOARD_SQUARE_COUNT];
extern const int king_endgame[BOARD_SQUARE_COUNT];

Score evaluate_position(const Board *pos);
bool  is_material_draw(const Board *pos);

#endif /* PEACE0X_EVALUATE_H */
