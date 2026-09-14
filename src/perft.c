#include "peace0x/perft.h"
#include "peace0x/board.h"
#include "peace0x/move.h"
#include "peace0x/movegen.h"
#include "peace0x/makemove.h"
#include "peace0x/platform.h"
#include <stdio.h>

uint64_t perft(int depth, Board *pos) {
    ASSERT(board_check(pos));

    if (depth == 0) {
        return 1ULL;
    }

    MoveList list;
    generate_all_moves(pos, &list);

    uint64_t nodes = 0ULL;

    for (int i = 0; i < list.count; i++) {
        if (!make_move(pos, list.moves[i].move)) {
            continue;
        }
        nodes += perft(depth - 1, pos);
        take_move(pos);
    }

    return nodes;
}

void perft_test(int depth, Board *pos) {
    ASSERT(board_check(pos));

    board_print(pos);
    printf("\nStarting Perft to Depth: %d\n", depth);

    int64_t start = platform_get_time_ms();
    uint64_t total_nodes = 0ULL;
    char move_buf[6];

    MoveList list;
    generate_all_moves(pos, &list);

    for (int i = 0; i < list.count; i++) {
        Move m = list.moves[i].move;
        if (!make_move(pos, m)) {
            continue;
        }

        uint64_t nodes = perft(depth - 1, pos);
        take_move(pos);

        total_nodes += nodes;
        move_to_string(m, move_buf);
        printf("Move %2d: %s : %llu\n", i + 1, move_buf, (unsigned long long)nodes);
    }

    int64_t elapsed = platform_get_time_ms() - start;
    if (elapsed == 0) elapsed = 1;
    uint64_t nps = (total_nodes * 1000ULL) / (uint64_t)elapsed;

    printf("\nPerft Complete: %llu nodes in %lld ms (%llu nps)\n",
           (unsigned long long)total_nodes, (long long)elapsed, (unsigned long long)nps);
}

typedef struct PerftTestCase {
    const char *name;
    const char *fen;
    int         depth;
    uint64_t    expected_nodes;
} PerftTestCase;

int run_perft_suite(void) {
    static const PerftTestCase cases[] = {
        /* Standard Initial Position */
        { "Startpos D1", START_FEN, 1, 20ULL },
        { "Startpos D2", START_FEN, 2, 400ULL },
        { "Startpos D3", START_FEN, 3, 8902ULL },
        { "Startpos D4", START_FEN, 4, 197281ULL },

        /* Kiwipete - Complex tactically rich position */
        { "Kiwipete D1", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 1, 48ULL },
        { "Kiwipete D2", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 2, 2039ULL },
        { "Kiwipete D3", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 3, 97862ULL },

        /* Position 3 */
        { "Position 3 D1", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 1, 14ULL },
        { "Position 3 D2", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 2, 191ULL },
        { "Position 3 D3", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 3, 2812ULL },
        { "Position 3 D4", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 4, 43238ULL },

        /* Position 4 - Mirrored/Castling nuances */
        { "Position 4 D1", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 1, 6ULL },
        { "Position 4 D2", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 2, 264ULL },
        { "Position 4 D3", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 3, 9467ULL },

        /* Position 5 */
        { "Position 5 D1", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 1, 44ULL },
        { "Position 5 D2", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 2, 1486ULL },
        { "Position 5 D3", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 3, 62379ULL },

        { NULL, NULL, 0, 0ULL }
    };

    printf("========================================\n");
    printf("   peace0x Perft Suite        \n");
    printf("========================================\n");

    Board pos;
    board_init(&pos);

    int passed = 0;
    int total  = 0;
    int64_t suite_start = platform_get_time_ms();

    for (int i = 0; cases[i].name != NULL; i++) {
        total++;
        board_parse_fen(cases[i].fen, &pos);

        int64_t test_start = platform_get_time_ms();
        uint64_t actual = perft(cases[i].depth, &pos);
        int64_t test_time = platform_get_time_ms() - test_start;

        if (actual == cases[i].expected_nodes) {
            passed++;
            printf("[PASS] %-16s | Nodes: %-10llu | Time: %3lld ms\n",
                   cases[i].name, (unsigned long long)actual, (long long)test_time);
        } else {
            printf("[FAIL] %-16s | Expected: %llu | Got: %llu\n",
                   cases[i].name,
                   (unsigned long long)cases[i].expected_nodes,
                   (unsigned long long)actual);
        }
    }

    int64_t total_time = platform_get_time_ms() - suite_start;
    printf("----------------------------------------\n");
    printf("Results: %d/%d passed in %lld ms\n", passed, total, (long long)total_time);
    printf("========================================\n");

    return (passed == total) ? 0 : 1;
}
