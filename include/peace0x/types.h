#ifndef PEACE0X_TYPES_H
#define PEACE0X_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ENGINE_NAME "peace0x 2.0"
#define ENGINE_AUTHOR "Naman Thanki"

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

/* Board and Search Limits */
#define SQUARE_COUNT        120
#define BOARD_SQUARE_COUNT  64
#define MAX_GAME_MOVES      2048
#define MAX_POSITION_MOVES  256
#define MAX_DEPTH           64

#define INFINITY_SCORE      30000
#define MATE_SCORE          29000
#define IS_MATE             (INFINITY_SCORE - MAX_DEPTH)

/* Custom assertion macro */
#ifdef DEBUG
#define ASSERT(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "Assertion failed: %s (%s: %s: %d)\n", \
                #expr, __FILE__, __func__, __LINE__); \
        abort(); \
    } \
} while (0)
#else
#define ASSERT(expr) ((void)0)
#endif

/* Primitive types */
typedef uint64_t Bitboard;
typedef uint32_t Move;
typedef int32_t  Score;

/* Piece definitions */
typedef enum Piece {
    PIECE_EMPTY = 0,
    PIECE_W_PAWN,
    PIECE_W_KNIGHT,
    PIECE_W_BISHOP,
    PIECE_W_ROOK,
    PIECE_W_QUEEN,
    PIECE_W_KING,
    PIECE_B_PAWN,
    PIECE_B_KNIGHT,
    PIECE_B_BISHOP,
    PIECE_B_ROOK,
    PIECE_B_QUEEN,
    PIECE_B_KING,
    PIECE_COUNT = 13
} Piece;

/* Color definitions */
typedef enum Color {
    COLOR_WHITE = 0,
    COLOR_BLACK = 1,
    COLOR_BOTH  = 2
} Color;

/* Files and Ranks */
typedef enum File {
    FILE_A = 0,
    FILE_B = 1,
    FILE_C = 2,
    FILE_D = 3,
    FILE_E = 4,
    FILE_F = 5,
    FILE_G = 6,
    FILE_H = 7,
    FILE_NONE = 8
} File;

typedef enum Rank {
    RANK_1 = 0,
    RANK_2 = 1,
    RANK_3 = 2,
    RANK_4 = 3,
    RANK_5 = 4,
    RANK_6 = 5,
    RANK_7 = 6,
    RANK_8 = 7,
    RANK_NONE = 8
} Rank;

/* 120-square mailbox coordinates */
typedef enum Square {
    SQ_A1 = 21, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
    SQ_A2 = 31, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
    SQ_A3 = 41, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
    SQ_A4 = 51, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
    SQ_A5 = 61, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
    SQ_A6 = 71, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
    SQ_A7 = 81, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
    SQ_A8 = 91, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
    SQ_NONE = 99,
    SQ_OFFBOARD = 100
} Square;

/* Castling permissions (4-bit representation) */
typedef enum CastlePerm {
    CASTLE_WK = 1,
    CASTLE_WQ = 2,
    CASTLE_BK = 4,
    CASTLE_BQ = 8
} CastlePerm;

/* Transposition Table entry flags */
typedef enum HashFlag {
    HASH_FLAG_NONE  = 0,
    HASH_FLAG_ALPHA = 1,
    HASH_FLAG_BETA  = 2,
    HASH_FLAG_EXACT = 3
} HashFlag;

/* Move scoring structure */
typedef struct ScoredMove {
    Move  move;
    Score score;
} ScoredMove;

/* Move list for generation */
typedef struct MoveList {
    ScoredMove moves[MAX_POSITION_MOVES];
    int        count;
} MoveList;

/* Undo state for making/taking moves */
typedef struct UndoState {
    Move     move;
    int      castle_perm;
    Square   en_passant;
    int      fifty_move;
    uint64_t hash_key;
} UndoState;

/* Transposition table entry */
typedef struct HashEntry {
    uint64_t hash_key;
    Move     move;
    Score    score;
    int      depth;
    HashFlag flags;
} HashEntry;

/* Transposition table structure */
typedef struct HashTable {
    HashEntry *entries;
    int        count;
    int        new_write;
    int        overwrite;
    int        hit;
    int        cut;
} HashTable;

/* Board representation structure */
typedef struct Board {
    Piece     pieces[SQUARE_COUNT];
    int       piece_count[PIECE_COUNT];
    int       big_pieces[2];
    int       major_pieces[2];
    int       minor_pieces[2];
    int       material[2];
    Square    king_square[2];
    Square    piece_list[PIECE_COUNT][10];
    Move      pv_array[MAX_DEPTH];
    int       search_history[PIECE_COUNT][SQUARE_COUNT];
    Move      search_killers[2][MAX_DEPTH];

    Color     side;
    Square    en_passant;
    int       fifty_move;
    int       ply;
    int       history_ply;
    int       castle_perm;

    Bitboard  pawns[3];
    uint64_t  hash_key;

    UndoState history[MAX_GAME_MOVES];
    HashTable hash_table;
} Board;

/* Search information and limits */
typedef struct SearchInfo {
    int64_t  start_time_ms;
    int64_t  stop_time_ms;
    int      depth;
    int      depth_set;
    bool     time_set;
    bool     quit;
    bool     stopped;
    int      moves_to_go;
    bool     infinite;

    uint64_t nodes;
    float    fail_high;
    float    fail_high_first;
    int      null_cut;
} SearchInfo;

#endif /* PEACE0X_TYPES_H */
