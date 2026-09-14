#ifndef PEACE0X_MAKEMOVE_H
#define PEACE0X_MAKEMOVE_H

#include "peace0x/types.h"

bool make_move(Board *pos, Move move);
void take_move(Board *pos);
void make_null_move(Board *pos);
void take_null_move(Board *pos);

#endif /* PEACE0X_MAKEMOVE_H */
