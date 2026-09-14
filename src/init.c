#include "peace0x/init.h"
#include "peace0x/board.h"
#include "peace0x/bitboard.h"
#include "peace0x/transposition.h"
#include "peace0x/movegen.h"

static void init_square_mappings(void) {
    for (int i = 0; i < SQUARE_COUNT; i++) {
        sq120_to_sq64[i] = 65;
    }

    for (int i = 0; i < BOARD_SQUARE_COUNT; i++) {
        sq64_to_sq120[i] = (Square)120;
    }

    int s64 = 0;
    for (int rank = RANK_1; rank <= RANK_8; rank++) {
        for (int file = FILE_A; file <= FILE_H; file++) {
            Square sq = file_rank_to_sq((File)file, (Rank)rank);
            sq64_to_sq120[s64] = sq;
            sq120_to_sq64[sq]  = s64;
            s64++;
        }
    }
}

static void init_files_ranks(void) {
    for (int i = 0; i < SQUARE_COUNT; i++) {
        files_board[i] = (int)SQ_OFFBOARD;
        ranks_board[i] = (int)SQ_OFFBOARD;
    }

    for (int rank = RANK_1; rank <= RANK_8; rank++) {
        for (int file = FILE_A; file <= FILE_H; file++) {
            Square sq = file_rank_to_sq((File)file, (Rank)rank);
            files_board[sq] = file;
            ranks_board[sq] = rank;
        }
    }
}

void all_init(void) {
    init_square_mappings();
    init_files_ranks();
    bitboards_init();
    hash_keys_init();
    movegen_init();
}
