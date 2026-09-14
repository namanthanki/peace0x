#include "peace0x/move.h"
#include "peace0x/board.h"
#include "peace0x/movegen.h"
#include <stdio.h>

void square_to_string(Square sq, char *out) {
    ASSERT(out != NULL);
    if (!square_on_board(sq)) {
        out[0] = '-';
        out[1] = '-';
        out[2] = '\0';
        return;
    }
    int file = files_board[sq];
    int rank = ranks_board[sq];
    out[0] = (char)('a' + file);
    out[1] = (char)('1' + rank);
    out[2] = '\0';
}

void move_to_string(Move move, char *out) {
    ASSERT(out != NULL);

    if (move == MOVE_NONE) {
        snprintf(out, 6, "none");
        return;
    }

    Square from = move_from(move);
    Square to   = move_to(move);

    int file_from = files_board[from];
    int rank_from = ranks_board[from];
    int file_to   = files_board[to];
    int rank_to   = ranks_board[to];

    Piece promoted = move_promoted(move);

    if (promoted != PIECE_EMPTY) {
        char promo_char = 'q';
        if (piece_knight[promoted]) {
            promo_char = 'n';
        } else if (piece_rook_queen[promoted] && !piece_bishop_queen[promoted]) {
            promo_char = 'r';
        } else if (!piece_rook_queen[promoted] && piece_bishop_queen[promoted]) {
            promo_char = 'b';
        }
        snprintf(out, 6, "%c%c%c%c%c",
                 'a' + file_from, '1' + rank_from,
                 'a' + file_to,   '1' + rank_to,
                 promo_char);
    } else {
        snprintf(out, 6, "%c%c%c%c",
                 'a' + file_from, '1' + rank_from,
                 'a' + file_to,   '1' + rank_to);
    }
}

Move move_parse(const char *str, Board *pos) {
    ASSERT(str != NULL);
    ASSERT(pos != NULL);

    if (str[0] < 'a' || str[0] > 'h') return MOVE_NONE;
    if (str[1] < '1' || str[1] > '8') return MOVE_NONE;
    if (str[2] < 'a' || str[2] > 'h') return MOVE_NONE;
    if (str[3] < '1' || str[3] > '8') return MOVE_NONE;

    Square from = file_rank_to_sq((File)(str[0] - 'a'), (Rank)(str[1] - '1'));
    Square to   = file_rank_to_sq((File)(str[2] - 'a'), (Rank)(str[3] - '1'));

    ASSERT(square_on_board(from) && square_on_board(to));

    MoveList list;
    generate_all_moves(pos, &list);

    for (int i = 0; i < list.count; i++) {
        Move m = list.moves[i].move;
        if (move_from(m) == from && move_to(m) == to) {
            Piece promoted = move_promoted(m);
            if (promoted != PIECE_EMPTY) {
                if (piece_rook_queen[promoted] && !piece_bishop_queen[promoted] && str[4] == 'r') {
                    return m;
                }
                if (!piece_rook_queen[promoted] && piece_bishop_queen[promoted] && str[4] == 'b') {
                    return m;
                }
                if (piece_rook_queen[promoted] && piece_bishop_queen[promoted] && str[4] == 'q') {
                    return m;
                }
                if (piece_knight[promoted] && str[4] == 'n') {
                    return m;
                }
                continue;
            }
            return m;
        }
    }

    return MOVE_NONE;
}

void move_list_print(const MoveList *list) {
    ASSERT(list != NULL);

    char move_buf[6];
    printf("MoveList (%d moves):\n", list->count);

    for (int i = 0; i < list->count; i++) {
        move_to_string(list->moves[i].move, move_buf);
        printf("Move %2d: %s (score: %d)\n", i + 1, move_buf, list->moves[i].score);
    }
    printf("\n");
}
