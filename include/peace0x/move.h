#ifndef PEACE0X_MOVE_H
#define PEACE0X_MOVE_H

#include "peace0x/types.h"

/* Move flag bitmasks */
#define MOVE_FLAG_EN_PASSANT 0x40000U
#define MOVE_FLAG_PAWN_START 0x80000U
#define MOVE_FLAG_CASTLED    0x1000000U
#define MOVE_FLAG_CAPTURED   0x7C000U
#define MOVE_FLAG_PROMOTION  0xF00000U

#define MOVE_NONE 0U

/* Move constructors and accessors */
static inline Move move_create(Square from, Square to, Piece captured, Piece promoted, uint32_t flags) {
    return ((uint32_t)from) |
           (((uint32_t)to) << 7U) |
           (((uint32_t)captured) << 14U) |
           (((uint32_t)promoted) << 20U) |
           flags;
}

static inline Square move_from(Move m) {
    return (Square)(m & 0x7FU);
}

static inline Square move_to(Move m) {
    return (Square)((m >> 7U) & 0x7FU);
}

static inline Piece move_captured(Move m) {
    return (Piece)((m >> 14U) & 0x0FU);
}

static inline Piece move_promoted(Move m) {
    return (Piece)((m >> 20U) & 0x0FU);
}

static inline bool move_is_en_passant(Move m) {
    return (m & MOVE_FLAG_EN_PASSANT) != 0;
}

static inline bool move_is_pawn_start(Move m) {
    return (m & MOVE_FLAG_PAWN_START) != 0;
}

static inline bool move_is_castled(Move m) {
    return (m & MOVE_FLAG_CASTLED) != 0;
}

static inline bool move_is_capture(Move m) {
    return (m & MOVE_FLAG_CAPTURED) != 0;
}

static inline bool move_is_promotion(Move m) {
    return (m & MOVE_FLAG_PROMOTION) != 0;
}

/* String conversions and I/O */
void move_to_string(Move move, char *out);
void square_to_string(Square sq, char *out);
Move move_parse(const char *str, Board *pos);
void move_list_print(const MoveList *list);

#endif /* PEACE0X_MOVE_H */
