#include "peace0x/bitboard.h"
#include "peace0x/board.h"

Bitboard set_mask[BOARD_SQUARE_COUNT];
Bitboard clear_mask[BOARD_SQUARE_COUNT];

Bitboard file_bitboard_mask[8];
Bitboard rank_bitboard_mask[8];

Bitboard black_passed_mask[BOARD_SQUARE_COUNT];
Bitboard white_passed_mask[BOARD_SQUARE_COUNT];
Bitboard isolated_pawn_mask[BOARD_SQUARE_COUNT];

void bitboards_init(void) {
    for (int i = 0; i < BOARD_SQUARE_COUNT; i++) {
        set_mask[i] = 1ULL << i;
        clear_mask[i] = ~set_mask[i];
    }

    for (int i = 0; i < 8; i++) {
        file_bitboard_mask[i] = 0ULL;
        rank_bitboard_mask[i] = 0ULL;
    }

    for (int rank = RANK_8; rank >= RANK_1; rank--) {
        for (int file = FILE_A; file <= FILE_H; file++) {
            int sq = rank * 8 + file;
            file_bitboard_mask[file] |= (1ULL << sq);
            rank_bitboard_mask[rank] |= (1ULL << sq);
        }
    }

    for (int sq = 0; sq < BOARD_SQUARE_COUNT; sq++) {
        black_passed_mask[sq] = 0ULL;
        white_passed_mask[sq] = 0ULL;
        isolated_pawn_mask[sq] = 0ULL;
    }

    for (int sq = 0; sq < BOARD_SQUARE_COUNT; sq++) {
        int tmp_sq = sq + 8;
        while (tmp_sq < 64) {
            white_passed_mask[sq] |= (1ULL << tmp_sq);
            tmp_sq += 8;
        }

        tmp_sq = sq - 8;
        while (tmp_sq >= 0) {
            black_passed_mask[sq] |= (1ULL << tmp_sq);
            tmp_sq -= 8;
        }

        int file = sq % 8;
        if (file > FILE_A) {
            isolated_pawn_mask[sq] |= file_bitboard_mask[file - 1];

            tmp_sq = sq + 7;
            while (tmp_sq < 64) {
                white_passed_mask[sq] |= (1ULL << tmp_sq);
                tmp_sq += 8;
            }

            tmp_sq = sq - 9;
            while (tmp_sq >= 0) {
                black_passed_mask[sq] |= (1ULL << tmp_sq);
                tmp_sq -= 8;
            }
        }

        if (file < FILE_H) {
            isolated_pawn_mask[sq] |= file_bitboard_mask[file + 1];

            tmp_sq = sq + 9;
            while (tmp_sq < 64) {
                white_passed_mask[sq] |= (1ULL << tmp_sq);
                tmp_sq += 8;
            }

            tmp_sq = sq - 7;
            while (tmp_sq >= 0) {
                black_passed_mask[sq] |= (1ULL << tmp_sq);
                tmp_sq -= 8;
            }
        }
    }
}

void bitboard_print(Bitboard bb) {
    printf("\n");
    for (int rank = RANK_8; rank >= RANK_1; rank--) {
        for (int file = FILE_A; file <= FILE_H; file++) {
            int sq64_idx = rank * 8 + file;
            if ((1ULL << sq64_idx) & bb) {
                printf("X ");
            } else {
                printf("- ");
            }
        }
        printf("\n");
    }
    printf("\n\n");
}
