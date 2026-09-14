#include "peace0x/uci.h"
#include "peace0x/board.h"
#include "peace0x/search.h"
#include "peace0x/move.h"
#include "peace0x/makemove.h"
#include "peace0x/transposition.h"
#include "peace0x/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_BUFFER_SIZE 2400

static void parse_go(char *line, SearchInfo *info, Board *pos) {
    int depth       = -1;
    int moves_to_go = 30;
    int move_time   = -1;
    int time        = -1;
    int increment   = 0;
    char *ptr       = NULL;

    info->time_set = false;

    if ((ptr = strstr(line, "infinite")) != NULL) {
        info->infinite = true;
    }

    if ((ptr = strstr(line, "binc")) != NULL && pos->side == COLOR_BLACK) {
        increment = atoi(ptr + 5);
    }
    if ((ptr = strstr(line, "winc")) != NULL && pos->side == COLOR_WHITE) {
        increment = atoi(ptr + 5);
    }

    if ((ptr = strstr(line, "wtime")) != NULL && pos->side == COLOR_WHITE) {
        time = atoi(ptr + 6);
    }
    if ((ptr = strstr(line, "btime")) != NULL && pos->side == COLOR_BLACK) {
        time = atoi(ptr + 6);
    }

    if ((ptr = strstr(line, "movestogo")) != NULL) {
        moves_to_go = atoi(ptr + 10);
    }

    if ((ptr = strstr(line, "movetime")) != NULL) {
        move_time = atoi(ptr + 9);
    }

    if ((ptr = strstr(line, "depth")) != NULL) {
        depth = atoi(ptr + 6);
    }

    if (move_time != -1) {
        time = move_time;
        moves_to_go = 1;
    }

    info->start_time_ms = platform_get_time_ms();
    info->depth         = depth;

    if (time != -1) {
        info->time_set = true;
        time /= moves_to_go;
        time -= 50;
        if (time < 0) time = 0;
        info->stop_time_ms = info->start_time_ms + time + increment;
    }

    if (depth == -1) {
        info->depth = MAX_DEPTH;
    }

    search_position(pos, info);
}

static void parse_position(char *line_in, Board *pos) {
    char *ptr = line_in + 8; /* skip "position" */
    while (*ptr == ' ') ptr++;

    if (strncmp(ptr, "startpos", 8) == 0) {
        board_parse_fen(START_FEN, pos);
    } else {
        char *fen_ptr = strstr(ptr, "fen");
        if (fen_ptr == NULL) {
            board_parse_fen(START_FEN, pos);
        } else {
            fen_ptr += 4;
            board_parse_fen(fen_ptr, pos);
        }
    }

    char *moves_ptr = strstr(ptr, "moves");
    if (moves_ptr != NULL) {
        moves_ptr += 5;
        while (*moves_ptr) {
            while (*moves_ptr == ' ') moves_ptr++;
            if (*moves_ptr == '\0') break;

            Move move = move_parse(moves_ptr, pos);
            if (move == MOVE_NONE) {
                break;
            }
            make_move(pos, move);
            pos->ply = 0;

            while (*moves_ptr && *moves_ptr != ' ') {
                moves_ptr++;
            }
        }
    }
}

void uci_loop(void) {
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    char line[INPUT_BUFFER_SIZE];
    printf("id name %s\n", ENGINE_NAME);
    printf("id author %s\n", ENGINE_AUTHOR);
    printf("uciok\n");
    fflush(stdout);

    Board pos;
    board_init(&pos);

    SearchInfo info;
    memset(&info, 0, sizeof(SearchInfo));

    /* 16 MB Hash Table by default */
    hash_table_init(&pos.hash_table, 0x100000 * 16);

    while (true) {
        memset(line, 0, sizeof(line));
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        if (line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        if (!strncmp(line, "isready", 7)) {
            printf("readyok\n");
            fflush(stdout);
            continue;
        } else if (!strncmp(line, "position", 8)) {
            parse_position(line, &pos);
        } else if (!strncmp(line, "ucinewgame", 10) || !strncmp(line, "uncinewgame", 11)) {
            hash_table_clear(&pos.hash_table);
            parse_position("position startpos\n", &pos);
        } else if (!strncmp(line, "go", 2)) {
            parse_go(line, &info, &pos);
        } else if (!strncmp(line, "stop", 4)) {
            /* Already idle, ignore */
            continue;
        } else if (!strncmp(line, "setoption", 9)) {
            /* Unsupported options safely ignored */
            continue;
        } else if (!strncmp(line, "quit", 4)) {
            info.quit = true;
            break;
        } else if (!strncmp(line, "uci", 3)) {
            printf("id name %s\n", ENGINE_NAME);
            printf("id author %s\n", ENGINE_AUTHOR);
            printf("uciok\n");
            fflush(stdout);
        }

        if (info.quit) {
            break;
        }
    }

    hash_table_free(&pos.hash_table);
}
