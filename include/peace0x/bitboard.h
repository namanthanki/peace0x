#ifndef PEACE0X_BITBOARD_H
#define PEACE0X_BITBOARD_H

#include "peace0x/types.h"

#if defined(_MSC_VER)
#include <intrin.h>
#endif

extern Bitboard set_mask[BOARD_SQUARE_COUNT];
extern Bitboard clear_mask[BOARD_SQUARE_COUNT];

extern Bitboard file_bitboard_mask[8];
extern Bitboard rank_bitboard_mask[8];

extern Bitboard black_passed_mask[BOARD_SQUARE_COUNT];
extern Bitboard white_passed_mask[BOARD_SQUARE_COUNT];
extern Bitboard isolated_pawn_mask[BOARD_SQUARE_COUNT];

void bitboards_init(void);
void bitboard_print(Bitboard bb);

static inline int bitboard_count_bits(Bitboard bb) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(bb);
#elif defined(_MSC_VER)
    return (int)__popcnt64(bb);
#else
    int count = 0;
    for (; bb; count++, bb &= bb - 1) {}
    return count;
#endif
}

static inline int bitboard_pop_bit(Bitboard *bb) {
    ASSERT(bb != NULL && *bb != 0ULL);
#if defined(__GNUC__) || defined(__clang__)
    int index = __builtin_ctzll(*bb);
    *bb &= (*bb - 1ULL);
    return index;
#elif defined(_MSC_VER)
    unsigned long index;
    _BitScanForward64(&index, *bb);
    *bb &= (*bb - 1ULL);
    return (int)index;
#else
    static const int de_bruijn_table[64] = {
        63, 30, 3, 32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34, 61, 29, 2,
        51, 21, 43, 45, 10, 18, 47, 1, 54, 9, 57, 0, 35, 62, 31, 40, 4, 49, 5, 52,
        26, 60, 6, 23, 44, 46, 27, 56, 16, 7, 39, 48, 24, 59, 14, 12, 55, 38, 28,
        58, 20, 37, 17, 36, 8
    };
    Bitboard bit = *bb ^ (*bb - 1ULL);
    unsigned int fold = (unsigned int)((bit & 0xFFFFFFFFU) ^ (bit >> 32U));
    *bb &= (*bb - 1ULL);
    return de_bruijn_table[(fold * 0x783A9B23U) >> 26U];
#endif
}

#endif /* PEACE0X_BITBOARD_H */
