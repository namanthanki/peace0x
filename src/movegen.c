#include "peace0x/movegen.h"
#include "peace0x/board.h"
#include "peace0x/move.h"
#include "peace0x/attack.h"
#include "peace0x/makemove.h"

static const Piece loop_slide_pieces[8] = {
    PIECE_W_BISHOP, PIECE_W_ROOK, PIECE_W_QUEEN, PIECE_EMPTY,
    PIECE_B_BISHOP, PIECE_B_ROOK, PIECE_B_QUEEN, PIECE_EMPTY
};

static const Piece loop_non_slide_pieces[6] = {
    PIECE_W_KNIGHT, PIECE_W_KING, PIECE_EMPTY,
    PIECE_B_KNIGHT, PIECE_B_KING, PIECE_EMPTY
};

static const int loop_slide_index[2]     = { 0, 4 };
static const int loop_non_slide_index[2] = { 0, 3 };

static const int piece_direction[13][8] = {
    {  0,   0,   0,   0,  0,  0,  0,  0 },
    {  0,   0,   0,   0,  0,  0,  0,  0 },
    { -8, -19, -21, -12,  8, 19, 21, 12 },
    { -9, -11,  11,   9,  0,  0,  0,  0 },
    { -1, -10,   1,  10,  0,  0,  0,  0 },
    { -1, -10,   1,  10, -9, -11, 11, 9 },
    { -1, -10,   1,  10, -9, -11, 11, 9 },
    {  0,   0,   0,   0,  0,  0,  0,  0 },
    { -8, -19, -21, -12,  8, 19, 21, 12 },
    { -9, -11,  11,   9,  0,  0,  0,  0 },
    { -1, -10,   1,  10,  0,  0,  0,  0 },
    { -1, -10,   1,  10, -9, -11, 11, 9 },
    { -1, -10,   1,  10, -9, -11, 11, 9 }
};

static const int num_directions[13] = {
    0, 0, 8, 4, 4, 8, 8, 0, 8, 4, 4, 8, 8
};

static const int victim_score[13] = {
    0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600
};

static int mvv_lva_scores[13][13];

void movegen_init(void) {
    for (int attacker = PIECE_W_PAWN; attacker <= PIECE_B_KING; attacker++) {
        for (int victim = PIECE_W_PAWN; victim <= PIECE_B_KING; victim++) {
            mvv_lva_scores[victim][attacker] = victim_score[victim] + 6 - (victim_score[attacker] / 100);
        }
    }
}

static void add_quiet_move(const Board *pos, Move move, MoveList *list) {
    ASSERT(square_on_board(move_from(move)));
    ASSERT(square_on_board(move_to(move)));

    list->moves[list->count].move = move;

    if (pos->search_killers[0][pos->ply] == move) {
        list->moves[list->count].score = 900000;
    } else if (pos->search_killers[1][pos->ply] == move) {
        list->moves[list->count].score = 800000;
    } else {
        list->moves[list->count].score = pos->search_history[pos->pieces[move_from(move)]][move_to(move)];
    }

    list->count++;
}

static void add_capture_move(const Board *pos, Move move, MoveList *list) {
    ASSERT(square_on_board(move_from(move)));
    ASSERT(square_on_board(move_to(move)));
    ASSERT(piece_valid(move_captured(move)));

    list->moves[list->count].move = move;
    list->moves[list->count].score = mvv_lva_scores[move_captured(move)][pos->pieces[move_from(move)]] + 1000000;
    list->count++;
}

static void add_en_passant_move(Move move, MoveList *list) {
    ASSERT(square_on_board(move_from(move)));
    ASSERT(square_on_board(move_to(move)));

    list->moves[list->count].move = move;
    list->moves[list->count].score = 105 + 1000000;
    list->count++;
}

static void add_white_pawn_capture_move(const Board *pos, Square from, Square to, Piece cap, MoveList *list) {
    ASSERT(square_on_board(from));
    ASSERT(square_on_board(to));
    ASSERT(piece_valid(cap));

    if (ranks_board[from] == RANK_7) {
        add_capture_move(pos, move_create(from, to, cap, PIECE_W_QUEEN,  0), list);
        add_capture_move(pos, move_create(from, to, cap, PIECE_W_ROOK,   0), list);
        add_capture_move(pos, move_create(from, to, cap, PIECE_W_BISHOP, 0), list);
        add_capture_move(pos, move_create(from, to, cap, PIECE_W_KNIGHT, 0), list);
    } else {
        add_capture_move(pos, move_create(from, to, cap, PIECE_EMPTY, 0), list);
    }
}

static void add_black_pawn_capture_move(const Board *pos, Square from, Square to, Piece cap, MoveList *list) {
    ASSERT(square_on_board(from));
    ASSERT(square_on_board(to));
    ASSERT(piece_valid(cap));

    if (ranks_board[from] == RANK_2) {
        add_capture_move(pos, move_create(from, to, cap, PIECE_B_QUEEN,  0), list);
        add_capture_move(pos, move_create(from, to, cap, PIECE_B_ROOK,   0), list);
        add_capture_move(pos, move_create(from, to, cap, PIECE_B_BISHOP, 0), list);
        add_capture_move(pos, move_create(from, to, cap, PIECE_B_KNIGHT, 0), list);
    } else {
        add_capture_move(pos, move_create(from, to, cap, PIECE_EMPTY, 0), list);
    }
}

static void add_white_pawn_quiet_move(const Board *pos, Square from, Square to, MoveList *list) {
    ASSERT(square_on_board(from));
    ASSERT(square_on_board(to));

    if (ranks_board[from] == RANK_7) {
        add_quiet_move(pos, move_create(from, to, PIECE_EMPTY, PIECE_W_QUEEN,  0), list);
        add_quiet_move(pos, move_create(from, to, PIECE_EMPTY, PIECE_W_ROOK,   0), list);
        add_quiet_move(pos, move_create(from, to, PIECE_EMPTY, PIECE_W_BISHOP, 0), list);
        add_quiet_move(pos, move_create(from, to, PIECE_EMPTY, PIECE_W_KNIGHT, 0), list);
    } else {
        add_quiet_move(pos, move_create(from, to, PIECE_EMPTY, PIECE_EMPTY, 0), list);
    }
}

static void add_black_pawn_quiet_move(const Board *pos, Square from, Square to, MoveList *list) {
    ASSERT(square_on_board(from));
    ASSERT(square_on_board(to));

    if (ranks_board[from] == RANK_2) {
        add_quiet_move(pos, move_create(from, to, PIECE_EMPTY, PIECE_B_QUEEN,  0), list);
        add_quiet_move(pos, move_create(from, to, PIECE_EMPTY, PIECE_B_ROOK,   0), list);
        add_quiet_move(pos, move_create(from, to, PIECE_EMPTY, PIECE_B_BISHOP, 0), list);
        add_quiet_move(pos, move_create(from, to, PIECE_EMPTY, PIECE_B_KNIGHT, 0), list);
    } else {
        add_quiet_move(pos, move_create(from, to, PIECE_EMPTY, PIECE_EMPTY, 0), list);
    }
}

void generate_all_moves(const Board *pos, MoveList *list) {
    ASSERT(board_check(pos));
    list->count = 0;

    Color side = pos->side;

    /* Pawns and Castling */
    if (side == COLOR_WHITE) {
        for (int i = 0; i < pos->piece_count[PIECE_W_PAWN]; i++) {
            Square sq = pos->piece_list[PIECE_W_PAWN][i];
            ASSERT(square_on_board(sq));

            /* Single push */
            if (pos->pieces[sq + 10] == PIECE_EMPTY) {
                add_white_pawn_quiet_move(pos, sq, (Square)(sq + 10), list);
                /* Double push */
                if (ranks_board[sq] == RANK_2 && pos->pieces[sq + 20] == PIECE_EMPTY) {
                    add_quiet_move(pos, move_create(sq, (Square)(sq + 20), PIECE_EMPTY, PIECE_EMPTY, MOVE_FLAG_PAWN_START), list);
                }
            }

            /* Captures */
            if (!square_on_board((Square)(sq + 9)) == false &&
                piece_color[pos->pieces[sq + 9]] == COLOR_BLACK) {
                add_white_pawn_capture_move(pos, sq, (Square)(sq + 9), pos->pieces[sq + 9], list);
            }
            if (!square_on_board((Square)(sq + 11)) == false &&
                piece_color[pos->pieces[sq + 11]] == COLOR_BLACK) {
                add_white_pawn_capture_move(pos, sq, (Square)(sq + 11), pos->pieces[sq + 11], list);
            }

            /* En Passant */
            if (pos->en_passant != SQ_NONE) {
                if (sq + 9 == pos->en_passant) {
                    add_en_passant_move(move_create(sq, (Square)(sq + 9), PIECE_EMPTY, PIECE_EMPTY, MOVE_FLAG_EN_PASSANT), list);
                }
                if (sq + 11 == pos->en_passant) {
                    add_en_passant_move(move_create(sq, (Square)(sq + 11), PIECE_EMPTY, PIECE_EMPTY, MOVE_FLAG_EN_PASSANT), list);
                }
            }
        }

        /* Castling White */
        if (pos->castle_perm & CASTLE_WK) {
            if (pos->pieces[SQ_F1] == PIECE_EMPTY && pos->pieces[SQ_G1] == PIECE_EMPTY) {
                if (!is_square_attacked(SQ_E1, COLOR_BLACK, pos) &&
                    !is_square_attacked(SQ_F1, COLOR_BLACK, pos)) {
                    add_quiet_move(pos, move_create(SQ_E1, SQ_G1, PIECE_EMPTY, PIECE_EMPTY, MOVE_FLAG_CASTLED), list);
                }
            }
        }
        if (pos->castle_perm & CASTLE_WQ) {
            if (pos->pieces[SQ_D1] == PIECE_EMPTY && pos->pieces[SQ_C1] == PIECE_EMPTY && pos->pieces[SQ_B1] == PIECE_EMPTY) {
                if (!is_square_attacked(SQ_E1, COLOR_BLACK, pos) &&
                    !is_square_attacked(SQ_D1, COLOR_BLACK, pos)) {
                    add_quiet_move(pos, move_create(SQ_E1, SQ_C1, PIECE_EMPTY, PIECE_EMPTY, MOVE_FLAG_CASTLED), list);
                }
            }
        }
    } else {
        for (int i = 0; i < pos->piece_count[PIECE_B_PAWN]; i++) {
            Square sq = pos->piece_list[PIECE_B_PAWN][i];
            ASSERT(square_on_board(sq));

            /* Single push */
            if (pos->pieces[sq - 10] == PIECE_EMPTY) {
                add_black_pawn_quiet_move(pos, sq, (Square)(sq - 10), list);
                /* Double push */
                if (ranks_board[sq] == RANK_7 && pos->pieces[sq - 20] == PIECE_EMPTY) {
                    add_quiet_move(pos, move_create(sq, (Square)(sq - 20), PIECE_EMPTY, PIECE_EMPTY, MOVE_FLAG_PAWN_START), list);
                }
            }

            /* Captures */
            if (!square_on_board((Square)(sq - 9)) == false &&
                piece_color[pos->pieces[sq - 9]] == COLOR_WHITE) {
                add_black_pawn_capture_move(pos, sq, (Square)(sq - 9), pos->pieces[sq - 9], list);
            }
            if (!square_on_board((Square)(sq - 11)) == false &&
                piece_color[pos->pieces[sq - 11]] == COLOR_WHITE) {
                add_black_pawn_capture_move(pos, sq, (Square)(sq - 11), pos->pieces[sq - 11], list);
            }

            /* En Passant */
            if (pos->en_passant != SQ_NONE) {
                if (sq - 9 == pos->en_passant) {
                    add_en_passant_move(move_create(sq, (Square)(sq - 9), PIECE_EMPTY, PIECE_EMPTY, MOVE_FLAG_EN_PASSANT), list);
                }
                if (sq - 11 == pos->en_passant) {
                    add_en_passant_move(move_create(sq, (Square)(sq - 11), PIECE_EMPTY, PIECE_EMPTY, MOVE_FLAG_EN_PASSANT), list);
                }
            }
        }

        /* Castling Black */
        if (pos->castle_perm & CASTLE_BK) {
            if (pos->pieces[SQ_F8] == PIECE_EMPTY && pos->pieces[SQ_G8] == PIECE_EMPTY) {
                if (!is_square_attacked(SQ_E8, COLOR_WHITE, pos) &&
                    !is_square_attacked(SQ_F8, COLOR_WHITE, pos)) {
                    add_quiet_move(pos, move_create(SQ_E8, SQ_G8, PIECE_EMPTY, PIECE_EMPTY, MOVE_FLAG_CASTLED), list);
                }
            }
        }
        if (pos->castle_perm & CASTLE_BQ) {
            if (pos->pieces[SQ_D8] == PIECE_EMPTY && pos->pieces[SQ_C8] == PIECE_EMPTY && pos->pieces[SQ_B8] == PIECE_EMPTY) {
                if (!is_square_attacked(SQ_E8, COLOR_WHITE, pos) &&
                    !is_square_attacked(SQ_D8, COLOR_WHITE, pos)) {
                    add_quiet_move(pos, move_create(SQ_E8, SQ_C8, PIECE_EMPTY, PIECE_EMPTY, MOVE_FLAG_CASTLED), list);
                }
            }
        }
    }

    /* Sliding pieces */
    int piece_idx = loop_slide_index[side];
    Piece piece = loop_slide_pieces[piece_idx++];
    while (piece != PIECE_EMPTY) {
        ASSERT(piece_valid(piece));
        for (int i = 0; i < pos->piece_count[piece]; i++) {
            Square sq = pos->piece_list[piece][i];
            ASSERT(square_on_board(sq));

            for (int d = 0; d < num_directions[piece]; d++) {
                int dir = piece_direction[piece][d];
                Square target = (Square)(sq + dir);

                while (pos->pieces[target] != (Piece)SQ_OFFBOARD) {
                    if (pos->pieces[target] != PIECE_EMPTY) {
                        if (piece_color[pos->pieces[target]] == (side ^ 1)) {
                            add_capture_move(pos, move_create(sq, target, pos->pieces[target], PIECE_EMPTY, 0), list);
                        }
                        break;
                    }
                    add_quiet_move(pos, move_create(sq, target, PIECE_EMPTY, PIECE_EMPTY, 0), list);
                    target = (Square)(target + dir);
                }
            }
        }
        piece = loop_slide_pieces[piece_idx++];
    }

    /* Non-sliding pieces */
    piece_idx = loop_non_slide_index[side];
    piece = loop_non_slide_pieces[piece_idx++];
    while (piece != PIECE_EMPTY) {
        ASSERT(piece_valid(piece));
        for (int i = 0; i < pos->piece_count[piece]; i++) {
            Square sq = pos->piece_list[piece][i];
            ASSERT(square_on_board(sq));

            for (int d = 0; d < num_directions[piece]; d++) {
                int dir = piece_direction[piece][d];
                Square target = (Square)(sq + dir);

                if (pos->pieces[target] == (Piece)SQ_OFFBOARD) continue;

                if (pos->pieces[target] != PIECE_EMPTY) {
                    if (piece_color[pos->pieces[target]] == (side ^ 1)) {
                        add_capture_move(pos, move_create(sq, target, pos->pieces[target], PIECE_EMPTY, 0), list);
                    }
                    continue;
                }
                add_quiet_move(pos, move_create(sq, target, PIECE_EMPTY, PIECE_EMPTY, 0), list);
            }
        }
        piece = loop_non_slide_pieces[piece_idx++];
    }
}

void generate_all_captures(const Board *pos, MoveList *list) {
    ASSERT(board_check(pos));
    list->count = 0;

    Color side = pos->side;

    if (side == COLOR_WHITE) {
        for (int i = 0; i < pos->piece_count[PIECE_W_PAWN]; i++) {
            Square sq = pos->piece_list[PIECE_W_PAWN][i];
            ASSERT(square_on_board(sq));

            if (!square_on_board((Square)(sq + 9)) == false &&
                piece_color[pos->pieces[sq + 9]] == COLOR_BLACK) {
                add_white_pawn_capture_move(pos, sq, (Square)(sq + 9), pos->pieces[sq + 9], list);
            }
            if (!square_on_board((Square)(sq + 11)) == false &&
                piece_color[pos->pieces[sq + 11]] == COLOR_BLACK) {
                add_white_pawn_capture_move(pos, sq, (Square)(sq + 11), pos->pieces[sq + 11], list);
            }

            if (pos->en_passant != SQ_NONE) {
                if (sq + 9 == pos->en_passant) {
                    add_en_passant_move(move_create(sq, (Square)(sq + 9), PIECE_EMPTY, PIECE_EMPTY, MOVE_FLAG_EN_PASSANT), list);
                }
                if (sq + 11 == pos->en_passant) {
                    add_en_passant_move(move_create(sq, (Square)(sq + 11), PIECE_EMPTY, PIECE_EMPTY, MOVE_FLAG_EN_PASSANT), list);
                }
            }
        }
    } else {
        for (int i = 0; i < pos->piece_count[PIECE_B_PAWN]; i++) {
            Square sq = pos->piece_list[PIECE_B_PAWN][i];
            ASSERT(square_on_board(sq));

            if (!square_on_board((Square)(sq - 9)) == false &&
                piece_color[pos->pieces[sq - 9]] == COLOR_WHITE) {
                add_black_pawn_capture_move(pos, sq, (Square)(sq - 9), pos->pieces[sq - 9], list);
            }
            if (!square_on_board((Square)(sq - 11)) == false &&
                piece_color[pos->pieces[sq - 11]] == COLOR_WHITE) {
                add_black_pawn_capture_move(pos, sq, (Square)(sq - 11), pos->pieces[sq - 11], list);
            }

            if (pos->en_passant != SQ_NONE) {
                if (sq - 9 == pos->en_passant) {
                    add_en_passant_move(move_create(sq, (Square)(sq - 9), PIECE_EMPTY, PIECE_EMPTY, MOVE_FLAG_EN_PASSANT), list);
                }
                if (sq - 11 == pos->en_passant) {
                    add_en_passant_move(move_create(sq, (Square)(sq - 11), PIECE_EMPTY, PIECE_EMPTY, MOVE_FLAG_EN_PASSANT), list);
                }
            }
        }
    }

    int piece_idx = loop_slide_index[side];
    Piece piece = loop_slide_pieces[piece_idx++];
    while (piece != PIECE_EMPTY) {
        ASSERT(piece_valid(piece));
        for (int i = 0; i < pos->piece_count[piece]; i++) {
            Square sq = pos->piece_list[piece][i];
            ASSERT(square_on_board(sq));

            for (int d = 0; d < num_directions[piece]; d++) {
                int dir = piece_direction[piece][d];
                Square target = (Square)(sq + dir);

                while (pos->pieces[target] != (Piece)SQ_OFFBOARD) {
                    if (pos->pieces[target] != PIECE_EMPTY) {
                        if (piece_color[pos->pieces[target]] == (side ^ 1)) {
                            add_capture_move(pos, move_create(sq, target, pos->pieces[target], PIECE_EMPTY, 0), list);
                        }
                        break;
                    }
                    target = (Square)(target + dir);
                }
            }
        }
        piece = loop_slide_pieces[piece_idx++];
    }

    piece_idx = loop_non_slide_index[side];
    piece = loop_non_slide_pieces[piece_idx++];
    while (piece != PIECE_EMPTY) {
        ASSERT(piece_valid(piece));
        for (int i = 0; i < pos->piece_count[piece]; i++) {
            Square sq = pos->piece_list[piece][i];
            ASSERT(square_on_board(sq));

            for (int d = 0; d < num_directions[piece]; d++) {
                int dir = piece_direction[piece][d];
                Square target = (Square)(sq + dir);

                if (pos->pieces[target] == (Piece)SQ_OFFBOARD) continue;

                if (pos->pieces[target] != PIECE_EMPTY) {
                    if (piece_color[pos->pieces[target]] == (side ^ 1)) {
                        add_capture_move(pos, move_create(sq, target, pos->pieces[target], PIECE_EMPTY, 0), list);
                    }
                }
            }
        }
        piece = loop_non_slide_pieces[piece_idx++];
    }
}

bool move_exists(Board *pos, Move move) {
    MoveList list;
    generate_all_moves(pos, &list);

    for (int i = 0; i < list.count; i++) {
        if (!make_move(pos, list.moves[i].move)) {
            continue;
        }
        take_move(pos);
        if (list.moves[i].move == move) {
            return true;
        }
    }
    return false;
}
