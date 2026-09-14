#include "peace0x/transposition.h"
#include "peace0x/board.h"
#include "peace0x/move.h"
#include "peace0x/movegen.h"
#include "peace0x/makemove.h"

uint64_t piece_keys[PIECE_COUNT][SQUARE_COUNT];
uint64_t side_key;
uint64_t castle_key[16];

static uint64_t rand_64(void) {
    return ((uint64_t)rand()) |
           (((uint64_t)rand()) << 15U) |
           (((uint64_t)rand()) << 30U) |
           (((uint64_t)rand()) << 45U) |
           ((((uint64_t)rand()) & 0x0FU) << 60U);
}

void hash_keys_init(void) {
    for (int p = 0; p < PIECE_COUNT; p++) {
        for (int sq = 0; sq < SQUARE_COUNT; sq++) {
            piece_keys[p][sq] = rand_64();
        }
    }
    side_key = rand_64();
    for (int i = 0; i < 16; i++) {
        castle_key[i] = rand_64();
    }
}

uint64_t generate_hash_key(const Board *pos) {
    ASSERT(pos != NULL);

    uint64_t key = 0ULL;

    for (int sq = 0; sq < SQUARE_COUNT; sq++) {
        Piece piece = pos->pieces[sq];
        if (piece != (Piece)SQ_OFFBOARD && piece != PIECE_EMPTY) {
            ASSERT(piece_valid(piece));
            key ^= piece_keys[piece][sq];
        }
    }

    if (pos->side == COLOR_WHITE) {
        key ^= side_key;
    }

    if (pos->en_passant != SQ_NONE) {
        ASSERT(square_on_board(pos->en_passant));
        key ^= piece_keys[PIECE_EMPTY][pos->en_passant];
    }

    ASSERT(pos->castle_perm >= 0 && pos->castle_perm <= 15);
    key ^= castle_key[pos->castle_perm];

    return key;
}

void hash_table_clear(HashTable *table) {
    ASSERT(table != NULL);
    if (table->entries != NULL) {
        memset(table->entries, 0, (size_t)table->count * sizeof(HashEntry));
    }
    table->new_write = 0;
    table->overwrite = 0;
    table->hit = 0;
    table->cut = 0;
}

void hash_table_init(HashTable *table, size_t size_bytes) {
    ASSERT(table != NULL);

    if (table->entries != NULL) {
        free(table->entries);
        table->entries = NULL;
    }

    table->count = (int)(size_bytes / sizeof(HashEntry));
    if (table->count > 2) {
        table->count -= 2;
    }

    table->entries = (HashEntry *)malloc((size_t)table->count * sizeof(HashEntry));
    if (table->entries == NULL) {
        fprintf(stderr, "Error: Failed to allocate transposition table (%zu bytes)\n", size_bytes);
        exit(EXIT_FAILURE);
    }

    hash_table_clear(table);
    printf("Transposition Table initialized with %d entries (%zu MB)\n",
           table->count, size_bytes / (1024 * 1024));
}

void hash_table_free(HashTable *table) {
    if (table != NULL && table->entries != NULL) {
        free(table->entries);
        table->entries = NULL;
        table->count = 0;
    }
}

void hash_table_store(Board *pos, Move move, Score score, HashFlag flags, int depth) {
    ASSERT(pos != NULL);
    HashTable *table = &pos->hash_table;
    if (table->entries == NULL || table->count == 0) return;

    int index = (int)(pos->hash_key % (uint64_t)table->count);
    ASSERT(index >= 0 && index < table->count);
    ASSERT(depth >= 1 && depth < MAX_DEPTH);
    ASSERT(flags >= HASH_FLAG_ALPHA && flags <= HASH_FLAG_EXACT);
    ASSERT(score >= -INFINITY_SCORE && score <= INFINITY_SCORE);
    ASSERT(pos->ply >= 0 && pos->ply < MAX_DEPTH);

    if (table->entries[index].hash_key == 0ULL) {
        table->new_write++;
    } else {
        table->overwrite++;
    }

    if (score > IS_MATE) {
        score += pos->ply;
    } else if (score < -IS_MATE) {
        score -= pos->ply;
    }

    table->entries[index].move     = move;
    table->entries[index].hash_key = pos->hash_key;
    table->entries[index].flags    = flags;
    table->entries[index].score    = score;
    table->entries[index].depth    = depth;
}

bool hash_table_probe(Board *pos, Move *move, Score *score, Score alpha, Score beta, int depth) {
    ASSERT(pos != NULL);
    ASSERT(move != NULL);
    ASSERT(score != NULL);

    HashTable *table = &pos->hash_table;
    if (table->entries == NULL || table->count == 0) return false;

    int index = (int)(pos->hash_key % (uint64_t)table->count);
    ASSERT(index >= 0 && index < table->count);

    if (table->entries[index].hash_key == pos->hash_key) {
        *move = table->entries[index].move;

        if (table->entries[index].depth >= depth) {
            table->hit++;

            Score entry_score = table->entries[index].score;
            if (entry_score > IS_MATE) {
                entry_score -= pos->ply;
            } else if (entry_score < -IS_MATE) {
                entry_score += pos->ply;
            }

            *score = entry_score;

            switch (table->entries[index].flags) {
                case HASH_FLAG_ALPHA:
                    if (entry_score <= alpha) {
                        *score = alpha;
                        return true;
                    }
                    break;
                case HASH_FLAG_BETA:
                    if (entry_score >= beta) {
                        *score = beta;
                        return true;
                    }
                    break;
                case HASH_FLAG_EXACT:
                    return true;
                default:
                    ASSERT(false);
                    break;
            }
        }
    }

    return false;
}

Move hash_table_probe_pv_move(const Board *pos) {
    ASSERT(pos != NULL);
    const HashTable *table = &pos->hash_table;
    if (table->entries == NULL || table->count == 0) return MOVE_NONE;

    int index = (int)(pos->hash_key % (uint64_t)table->count);
    if (table->entries[index].hash_key == pos->hash_key) {
        return table->entries[index].move;
    }
    return MOVE_NONE;
}

int get_pv_line(int depth, Board *pos, Move *pv_array) {
    ASSERT(pos != NULL);
    ASSERT(pv_array != NULL);
    ASSERT(depth < MAX_DEPTH);

    Move move = hash_table_probe_pv_move(pos);
    int count = 0;

    while (move != MOVE_NONE && count < depth) {
        ASSERT(count < MAX_DEPTH);

        if (move_exists(pos, move)) {
            make_move(pos, move);
            pv_array[count++] = move;
        } else {
            break;
        }
        move = hash_table_probe_pv_move(pos);
    }

    while (pos->ply > 0) {
        take_move(pos);
    }

    return count;
}
