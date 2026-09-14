#ifndef PEACE0X_PERFT_H
#define PEACE0X_PERFT_H

#include "peace0x/types.h"

uint64_t perft(int depth, Board *pos);
void     perft_test(int depth, Board *pos);
int      run_perft_suite(void);

#endif /* PEACE0X_PERFT_H */
