#ifndef PEACE0X_MOVEGEN_H
#define PEACE0X_MOVEGEN_H

#include "peace0x/types.h"

void movegen_init(void);
void generate_all_moves(const Board *pos, MoveList *list);
void generate_all_captures(const Board *pos, MoveList *list);
bool move_exists(Board *pos, Move move);

#endif /* PEACE0X_MOVEGEN_H */
