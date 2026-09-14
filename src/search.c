#include "peace0x/search.h"
#include "peace0x/board.h"
#include "peace0x/move.h"
#include "peace0x/movegen.h"
#include "peace0x/makemove.h"
#include "peace0x/evaluate.h"
#include "peace0x/transposition.h"
#include "peace0x/attack.h"
#include "peace0x/platform.h"
#include <stdio.h>
#include <stdlib.h>

static void check_up(SearchInfo *info) {
    if (info->time_set && platform_get_time_ms() > info->stop_time_ms) {
        info->stopped = true;
    }
    platform_read_input(info);
}

static void pick_next_move(int move_num, MoveList *list) {
    int best_score = list->moves[move_num].score;
    int best_num   = move_num;

    for (int i = move_num + 1; i < list->count; i++) {
        if (list->moves[i].score > best_score) {
            best_score = list->moves[i].score;
            best_num   = i;
        }
    }

    ScoredMove temp        = list->moves[move_num];
    list->moves[move_num]  = list->moves[best_num];
    list->moves[best_num]  = temp;
}

static bool is_repetition(const Board *pos) {
    int start = pos->history_ply - pos->fifty_move;
    if (start < 0) {
        start = 0;
    }
    for (int i = start; i < pos->history_ply - 1; i++) {
        ASSERT(i >= 0 && i < MAX_GAME_MOVES);
        if (pos->hash_key == pos->history[i].hash_key) {
            return true;
        }
    }
    return false;
}

static void clear_for_search(Board *pos, SearchInfo *info) {
    for (int i = 0; i < PIECE_COUNT; i++) {
        for (int j = 0; j < SQUARE_COUNT; j++) {
            pos->search_history[i][j] = 0;
        }
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < MAX_DEPTH; j++) {
            pos->search_killers[i][j] = MOVE_NONE;
        }
    }

    pos->hash_table.overwrite = 0;
    pos->hash_table.hit       = 0;
    pos->hash_table.cut       = 0;
    pos->ply                  = 0;

    info->stopped         = false;
    info->nodes           = 0ULL;
    info->fail_high       = 0.0f;
    info->fail_high_first = 0.0f;
}

static Score quiescence(Score alpha, Score beta, Board *pos, SearchInfo *info) {
    ASSERT(board_check(pos));

    if ((info->nodes & 2047ULL) == 0ULL) {
        check_up(info);
    }

    info->nodes++;

    if ((is_repetition(pos) || pos->fifty_move >= 100) && pos->ply) {
        return 0;
    }

    if (pos->ply > MAX_DEPTH - 1) {
        return evaluate_position(pos);
    }

    Score score = evaluate_position(pos);

    if (score >= beta) {
        return beta;
    }

    if (score > alpha) {
        alpha = score;
    }

    MoveList list;
    generate_all_captures(pos, &list);

    int legal = 0;

    for (int i = 0; i < list.count; i++) {
        pick_next_move(i, &list);

        if (!make_move(pos, list.moves[i].move)) {
            continue;
        }

        legal++;
        score = -quiescence(-beta, -alpha, pos, info);
        take_move(pos);

        if (info->stopped) {
            return 0;
        }

        if (score > alpha) {
            if (score >= beta) {
                if (legal == 1) {
                    info->fail_high_first++;
                }
                info->fail_high++;
                return beta;
            }
            alpha = score;
        }
    }

    return alpha;
}

static Score alpha_beta(Score alpha, Score beta, int depth, Board *pos, SearchInfo *info, bool do_null) {
    ASSERT(board_check(pos));

    if (depth == 0) {
        info->nodes++;
        return quiescence(alpha, beta, pos, info);
    }

    if ((info->nodes & 2047ULL) == 0ULL) {
        check_up(info);
    }

    info->nodes++;

    if ((is_repetition(pos) || pos->fifty_move >= 100) && pos->ply) {
        return 0;
    }

    if (pos->ply > MAX_DEPTH - 1) {
        return evaluate_position(pos);
    }

    bool in_check = is_square_attacked(pos->king_square[pos->side], (Color)(pos->side ^ 1), pos);
    if (in_check) {
        depth++;
    }

    Move pv_move = MOVE_NONE;
    Score score = -INFINITY_SCORE;

    /* Do NOT cutoff at the root (ply == 0); only probe for move ordering */
    if (pos->ply > 0) {
        if (hash_table_probe(pos, &pv_move, &score, alpha, beta, depth)) {
            pos->hash_table.cut++;
            return score;
        }
    } else {
        pv_move = hash_table_probe_pv_move(pos);
    }

    /* Null Move Pruning */
    if (do_null && !in_check && pos->ply && (pos->big_pieces[pos->side] >= 2) && depth >= 4) {
        make_null_move(pos);
        score = -alpha_beta(-beta, -beta + 1, depth - 4, pos, info, false);
        take_null_move(pos);

        if (info->stopped) {
            return 0;
        }
        if (score >= beta && abs(score) < IS_MATE) {
            info->null_cut++;
            return beta;
        }
    }

    MoveList list;
    generate_all_moves(pos, &list);

    int legal = 0;
    Score old_alpha  = alpha;
    Move  best_move  = MOVE_NONE;
    Score best_score = -INFINITY_SCORE;

    if (pv_move != MOVE_NONE) {
        for (int i = 0; i < list.count; i++) {
            if (list.moves[i].move == pv_move) {
                list.moves[i].score = 2000000;
                break;
            }
        }
    }

    for (int i = 0; i < list.count; i++) {
        pick_next_move(i, &list);

        if (!make_move(pos, list.moves[i].move)) {
            continue;
        }

        legal++;
        score = -alpha_beta(-beta, -alpha, depth - 1, pos, info, true);
        take_move(pos);

        if (info->stopped) {
            return 0;
        }

        if (score > best_score) {
            best_score = score;
            best_move  = list.moves[i].move;

            if (score > alpha) {
                if (score >= beta) {
                    if (legal == 1) {
                        info->fail_high_first++;
                    }
                    info->fail_high++;

                    if (!move_is_capture(list.moves[i].move)) {
                        pos->search_killers[1][pos->ply] = pos->search_killers[0][pos->ply];
                        pos->search_killers[0][pos->ply] = list.moves[i].move;
                    }

                    hash_table_store(pos, best_move, beta, HASH_FLAG_BETA, depth);
                    return beta;
                }
                alpha = score;
                if (!move_is_capture(list.moves[i].move)) {
                    pos->search_history[pos->pieces[move_from(best_move)]][move_to(best_move)] += depth;
                }
            }
        }
    }

    if (legal == 0) {
        if (in_check) {
            return -INFINITY_SCORE + pos->ply;
        } else {
            return 0;
        }
    }

    if (alpha != old_alpha) {
        hash_table_store(pos, best_move, best_score, HASH_FLAG_EXACT, depth);
    } else {
        hash_table_store(pos, best_move, alpha, HASH_FLAG_ALPHA, depth);
    }

    return alpha;
}

void search_position(Board *pos, SearchInfo *info) {
    ASSERT(pos != NULL);
    ASSERT(info != NULL);

    Move best_move  = MOVE_NONE;
    Score best_score = -INFINITY_SCORE;
    char move_buf[6];

    clear_for_search(pos, info);

    for (int depth = 1; depth <= info->depth; depth++) {
        best_score = alpha_beta(-INFINITY_SCORE, INFINITY_SCORE, depth, pos, info, true);

        if (info->stopped) {
            break;
        }

        int pv_moves = get_pv_line(depth, pos, pos->pv_array);
        if (pv_moves > 0) {
            best_move = pos->pv_array[0];
        }

        int64_t elapsed = platform_get_time_ms() - info->start_time_ms;
        uint64_t nps = (elapsed > 0) ? ((info->nodes * 1000ULL) / (uint64_t)elapsed) : 0ULL;

        if (best_score > IS_MATE) {
            int mate_in = (INFINITY_SCORE - best_score + 1) / 2;
            printf("info depth %d score mate %d time %lld nodes %llu nps %llu pv",
                   depth, mate_in, (long long)elapsed, (unsigned long long)info->nodes, (unsigned long long)nps);
        } else if (best_score < -IS_MATE) {
            int mate_in = -(INFINITY_SCORE + best_score + 1) / 2;
            printf("info depth %d score mate %d time %lld nodes %llu nps %llu pv",
                   depth, mate_in, (long long)elapsed, (unsigned long long)info->nodes, (unsigned long long)nps);
        } else {
            printf("info depth %d score cp %d time %lld nodes %llu nps %llu pv",
                   depth, best_score, (long long)elapsed, (unsigned long long)info->nodes, (unsigned long long)nps);
        }

        for (int i = 0; i < pv_moves; i++) {
            move_to_string(pos->pv_array[i], move_buf);
            printf(" %s", move_buf);
        }
        printf("\n");
        fflush(stdout);
    }

    move_to_string(best_move, move_buf);
    printf("bestmove %s\n", move_buf);
    fflush(stdout);
}
