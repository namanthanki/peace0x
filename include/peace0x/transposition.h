#ifndef PEACE0X_TRANSPOSITION_H
#define PEACE0X_TRANSPOSITION_H

#include "peace0x/types.h"

extern uint64_t piece_keys[PIECE_COUNT][SQUARE_COUNT];
extern uint64_t side_key;
extern uint64_t castle_key[16];

void     hash_keys_init(void);
uint64_t generate_hash_key(const Board *pos);

void hash_table_init(HashTable *table, size_t size_bytes);
void hash_table_clear(HashTable *table);
void hash_table_free(HashTable *table);
void hash_table_store(Board *pos, Move move, Score score, HashFlag flags, int depth);
bool hash_table_probe(Board *pos, Move *move, Score *score, Score alpha, Score beta, int depth);
Move hash_table_probe_pv_move(const Board *pos);
int  get_pv_line(int depth, Board *pos, Move *pv_array);

#endif /* PEACE0X_TRANSPOSITION_H */
