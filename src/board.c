#include "peace0x/board.h"
#include "peace0x/transposition.h"
#include <ctype.h>

int    sq120_to_sq64[SQUARE_COUNT];
Square sq64_to_sq120[BOARD_SQUARE_COUNT];
int    files_board[SQUARE_COUNT];
int    ranks_board[SQUARE_COUNT];

int mirror64[BOARD_SQUARE_COUNT] = {
    56, 57, 58, 59, 60, 61, 62, 63,
    48, 49, 50, 51, 52, 53, 54, 55,
    40, 41, 42, 43, 44, 45, 46, 47,
    32, 33, 34, 35, 36, 37, 38, 39,
    24, 25, 26, 27, 28, 29, 30, 31,
    16, 17, 18, 19, 20, 21, 22, 23,
     8,  9, 10, 11, 12, 13, 14, 15,
     0,  1,  2,  3,  4,  5,  6,  7
};

const char piece_char[] = ".PNBRQKpnbrqk";
const char side_char[]  = "wb-";
const char rank_char[]  = "12345678";
const char file_char[]  = "abcdefgh";

const bool piece_big[PIECE_COUNT]          = { false, false, true, true, true, true, true, false, true, true, true, true, true };
const bool piece_major[PIECE_COUNT]        = { false, false, false, false, true, true, true, false, false, false, true, true, true };
const bool piece_minor[PIECE_COUNT]        = { false, false, true, true, false, false, false, false, true, true, false, false, false };
const int  piece_value[PIECE_COUNT]        = { 0, 100, 325, 325, 550, 1000, 50000, 100, 325, 325, 550, 1000, 50000 };
const Color piece_color[PIECE_COUNT]       = { COLOR_BOTH, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK };
const bool piece_pawn[PIECE_COUNT]         = { false, true, false, false, false, false, false, true, false, false, false, false, false };
const bool piece_knight[PIECE_COUNT]       = { false, false, true, false, false, false, false, false, true, false, false, false, false };
const bool piece_king[PIECE_COUNT]         = { false, false, false, false, false, false, true, false, false, false, false, false, true };
const bool piece_rook_queen[PIECE_COUNT]   = { false, false, false, false, true, true, false, false, false, false, true, true, false };
const bool piece_bishop_queen[PIECE_COUNT] = { false, false, false, true, false, true, false, false, false, true, false, true, false };
const bool piece_slides[PIECE_COUNT]       = { false, false, false, true, true, true, false, false, false, true, true, true, false };

void board_reset(Board *pos) {
    ASSERT(pos != NULL);

    for (int i = 0; i < SQUARE_COUNT; i++) {
        pos->pieces[i] = (Piece)SQ_OFFBOARD;
    }

    for (int i = 0; i < BOARD_SQUARE_COUNT; i++) {
        pos->pieces[sq120(i)] = PIECE_EMPTY;
    }

    for (int i = 0; i < 2; i++) {
        pos->big_pieces[i]   = 0;
        pos->major_pieces[i] = 0;
        pos->minor_pieces[i] = 0;
        pos->material[i]     = 0;
    }

    for (int i = 0; i < 3; i++) {
        pos->pawns[i] = 0ULL;
    }

    for (int i = 0; i < PIECE_COUNT; i++) {
        pos->piece_count[i] = 0;
    }

    pos->king_square[COLOR_WHITE] = SQ_NONE;
    pos->king_square[COLOR_BLACK] = SQ_NONE;
    pos->side             = COLOR_BOTH;
    pos->en_passant       = SQ_NONE;
    pos->fifty_move       = 0;
    pos->ply              = 0;
    pos->history_ply      = 0;
    pos->castle_perm      = 0;
    pos->hash_key         = 0ULL;
}

void board_init(Board *pos) {
    ASSERT(pos != NULL);
    memset(pos, 0, sizeof(Board));
    board_reset(pos);
}

void board_update_lists_material(Board *pos) {
    for (int i = 0; i < SQUARE_COUNT; i++) {
        Square sq = (Square)i;
        Piece piece = pos->pieces[i];

        if (piece != (Piece)SQ_OFFBOARD && piece != PIECE_EMPTY) {
            Color color = piece_color[piece];

            if (piece_big[piece])   pos->big_pieces[color]++;
            if (piece_minor[piece]) pos->minor_pieces[color]++;
            if (piece_major[piece]) pos->major_pieces[color]++;

            pos->material[color] += piece_value[piece];
            pos->piece_list[piece][pos->piece_count[piece]] = sq;
            pos->piece_count[piece]++;

            if (piece == PIECE_W_KING) pos->king_square[COLOR_WHITE] = sq;
            if (piece == PIECE_B_KING) pos->king_square[COLOR_BLACK] = sq;

            if (piece == PIECE_W_PAWN) {
                pos->pawns[COLOR_WHITE] |= (1ULL << sq64(sq));
                pos->pawns[COLOR_BOTH]  |= (1ULL << sq64(sq));
            } else if (piece == PIECE_B_PAWN) {
                pos->pawns[COLOR_BLACK] |= (1ULL << sq64(sq));
                pos->pawns[COLOR_BOTH]  |= (1ULL << sq64(sq));
            }
        }
    }
}

bool board_check(const Board *pos) {
#ifndef DEBUG
    (void)pos;
    return true;
#else
    int tmp_piece_count[PIECE_COUNT] = { 0 };
    int tmp_big_piece[2]   = { 0, 0 };
    int tmp_major_piece[2] = { 0, 0 };
    int tmp_minor_piece[2] = { 0, 0 };
    int tmp_material[2]    = { 0, 0 };

    Bitboard tmp_pawns[3] = {
        pos->pawns[COLOR_WHITE],
        pos->pawns[COLOR_BLACK],
        pos->pawns[COLOR_BOTH]
    };

    for (int piece = PIECE_W_PAWN; piece <= PIECE_B_KING; piece++) {
        for (int num = 0; num < pos->piece_count[piece]; num++) {
            Square sq = pos->piece_list[piece][num];
            ASSERT(pos->pieces[sq] == piece);
        }
    }

    for (int s64 = 0; s64 < BOARD_SQUARE_COUNT; s64++) {
        Square sq = sq120(s64);
        Piece piece = pos->pieces[sq];
        tmp_piece_count[piece]++;
        Color color = piece_color[piece];

        if (piece_big[piece])   tmp_big_piece[color]++;
        if (piece_minor[piece]) tmp_minor_piece[color]++;
        if (piece_major[piece]) tmp_major_piece[color]++;

        tmp_material[color] += piece_value[piece];
    }

    for (int piece = PIECE_W_PAWN; piece <= PIECE_B_KING; piece++) {
        ASSERT(tmp_piece_count[piece] == pos->piece_count[piece]);
    }

    int pwn_cnt = bitboard_count_bits(tmp_pawns[COLOR_WHITE]);
    ASSERT(pwn_cnt == pos->piece_count[PIECE_W_PAWN]);
    pwn_cnt = bitboard_count_bits(tmp_pawns[COLOR_BLACK]);
    ASSERT(pwn_cnt == pos->piece_count[PIECE_B_PAWN]);
    pwn_cnt = bitboard_count_bits(tmp_pawns[COLOR_BOTH]);
    ASSERT(pwn_cnt == pos->piece_count[PIECE_W_PAWN] + pos->piece_count[PIECE_B_PAWN]);

    while (tmp_pawns[COLOR_WHITE]) {
        int s64 = bitboard_pop_bit(&tmp_pawns[COLOR_WHITE]);
        ASSERT(pos->pieces[sq120(s64)] == PIECE_W_PAWN);
    }
    while (tmp_pawns[COLOR_BLACK]) {
        int s64 = bitboard_pop_bit(&tmp_pawns[COLOR_BLACK]);
        ASSERT(pos->pieces[sq120(s64)] == PIECE_B_PAWN);
    }
    while (tmp_pawns[COLOR_BOTH]) {
        int s64 = bitboard_pop_bit(&tmp_pawns[COLOR_BOTH]);
        ASSERT((pos->pieces[sq120(s64)] == PIECE_B_PAWN) || (pos->pieces[sq120(s64)] == PIECE_W_PAWN));
    }

    ASSERT(tmp_material[COLOR_WHITE] == pos->material[COLOR_WHITE] && tmp_material[COLOR_BLACK] == pos->material[COLOR_BLACK]);
    ASSERT(tmp_minor_piece[COLOR_WHITE] == pos->minor_pieces[COLOR_WHITE] && tmp_minor_piece[COLOR_BLACK] == pos->minor_pieces[COLOR_BLACK]);
    ASSERT(tmp_major_piece[COLOR_WHITE] == pos->major_pieces[COLOR_WHITE] && tmp_major_piece[COLOR_BLACK] == pos->major_pieces[COLOR_BLACK]);
    ASSERT(tmp_big_piece[COLOR_WHITE] == pos->big_pieces[COLOR_WHITE] && tmp_big_piece[COLOR_BLACK] == pos->big_pieces[COLOR_BLACK]);

    ASSERT(pos->side == COLOR_WHITE || pos->side == COLOR_BLACK);
    ASSERT(generate_hash_key(pos) == pos->hash_key);

    ASSERT(pos->en_passant == SQ_NONE ||
           (ranks_board[pos->en_passant] == RANK_6 && pos->side == COLOR_WHITE) ||
           (ranks_board[pos->en_passant] == RANK_3 && pos->side == COLOR_BLACK));

    ASSERT(pos->pieces[pos->king_square[COLOR_WHITE]] == PIECE_W_KING);
    ASSERT(pos->pieces[pos->king_square[COLOR_BLACK]] == PIECE_B_KING);

    return true;
#endif
}

int board_parse_fen(const char *fen, Board *pos) {
    ASSERT(fen != NULL);
    ASSERT(pos != NULL);

    int rank = RANK_8;
    int file = FILE_A;
    int count = 0;

    board_reset(pos);

    while ((rank >= RANK_1) && *fen) {
        count = 1;
        Piece piece = PIECE_EMPTY;

        switch (*fen) {
            case 'p': piece = PIECE_B_PAWN;   break;
            case 'r': piece = PIECE_B_ROOK;   break;
            case 'n': piece = PIECE_B_KNIGHT; break;
            case 'b': piece = PIECE_B_BISHOP; break;
            case 'k': piece = PIECE_B_KING;   break;
            case 'q': piece = PIECE_B_QUEEN;  break;
            case 'P': piece = PIECE_W_PAWN;   break;
            case 'R': piece = PIECE_W_ROOK;   break;
            case 'N': piece = PIECE_W_KNIGHT; break;
            case 'B': piece = PIECE_W_BISHOP; break;
            case 'K': piece = PIECE_W_KING;   break;
            case 'Q': piece = PIECE_W_QUEEN;  break;

            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                piece = PIECE_EMPTY;
                count = *fen - '0';
                break;

            case '/':
            case ' ':
                rank--;
                file = FILE_A;
                fen++;
                continue;

            default:
                fprintf(stderr, "FEN error: invalid piece character '%c'\n", *fen);
                return -1;
        }

        for (int i = 0; i < count; i++) {
            Square sq = (Square)(21 + file + (rank * 10));
            if (piece != PIECE_EMPTY) {
                pos->pieces[sq] = piece;
            }
            file++;
        }
        fen++;
    }

    ASSERT(*fen == 'w' || *fen == 'b');
    pos->side = (*fen == 'w') ? COLOR_WHITE : COLOR_BLACK;
    fen += 2;

    for (int i = 0; i < 4; i++) {
        if (*fen == ' ') break;
        switch (*fen) {
            case 'K': pos->castle_perm |= CASTLE_WK; break;
            case 'Q': pos->castle_perm |= CASTLE_WQ; break;
            case 'k': pos->castle_perm |= CASTLE_BK; break;
            case 'q': pos->castle_perm |= CASTLE_BQ; break;
            default: break;
        }
        fen++;
    }
    fen++;

    ASSERT(pos->castle_perm >= 0 && pos->castle_perm <= 15);

    if (*fen != '-') {
        file = fen[0] - 'a';
        rank = fen[1] - '1';

        ASSERT(file >= FILE_A && file <= FILE_H);
        ASSERT(rank >= RANK_1 && rank <= RANK_8);

        pos->en_passant = file_rank_to_sq((File)file, (Rank)rank);
    }

    pos->hash_key = generate_hash_key(pos);
    board_update_lists_material(pos);
    return 0;
}

void board_print(const Board *pos) {
    ASSERT(pos != NULL);

    printf("\nGame Board\n\n");
    for (int rank = RANK_8; rank >= RANK_1; rank--) {
        printf("%d  ", rank + 1);
        for (int file = FILE_A; file <= FILE_H; file++) {
            Square sq = file_rank_to_sq((File)file, (Rank)rank);
            Piece piece = pos->pieces[sq];
            printf("%3c", piece_char[piece]);
        }
        printf("\n");
    }

    printf("\n   ");
    for (int file = FILE_A; file <= FILE_H; file++) {
        printf("%3c", 'a' + file);
    }
    printf("\n");
    printf("Side: %c\n", side_char[pos->side]);
    printf("En Passant: %d\n", pos->en_passant);
    printf("Castle: %c%c%c%c\n",
           (pos->castle_perm & CASTLE_WK) ? 'K' : '-',
           (pos->castle_perm & CASTLE_WQ) ? 'Q' : '-',
           (pos->castle_perm & CASTLE_BK) ? 'k' : '-',
           (pos->castle_perm & CASTLE_BQ) ? 'q' : '-');
    printf("Hash Key: %llx\n", (unsigned long long)pos->hash_key);
}

void board_mirror(Board *pos) {
    ASSERT(pos != NULL);

    Piece tmp_pieces_array[64];
    Color tmp_side = (Color)(pos->side ^ 1);
    static const Piece swap_piece[PIECE_COUNT] = {
        PIECE_EMPTY,
        PIECE_B_PAWN, PIECE_B_KNIGHT, PIECE_B_BISHOP, PIECE_B_ROOK, PIECE_B_QUEEN, PIECE_B_KING,
        PIECE_W_PAWN, PIECE_W_KNIGHT, PIECE_W_BISHOP, PIECE_W_ROOK, PIECE_W_QUEEN, PIECE_W_KING
    };

    int tmp_castle_perm = 0;
    Square tmp_en_passant = SQ_NONE;

    if (pos->castle_perm & CASTLE_WK) tmp_castle_perm |= CASTLE_BK;
    if (pos->castle_perm & CASTLE_WQ) tmp_castle_perm |= CASTLE_BQ;
    if (pos->castle_perm & CASTLE_BK) tmp_castle_perm |= CASTLE_WK;
    if (pos->castle_perm & CASTLE_BQ) tmp_castle_perm |= CASTLE_WQ;

    if (pos->en_passant != SQ_NONE) {
        tmp_en_passant = sq120(mirror_64(sq64(pos->en_passant)));
    }

    for (int sq = 0; sq < 64; sq++) {
        tmp_pieces_array[sq] = pos->pieces[sq120(mirror_64(sq))];
    }

    board_reset(pos);

    for (int sq = 0; sq < 64; sq++) {
        pos->pieces[sq120(sq)] = swap_piece[tmp_pieces_array[sq]];
    }

    pos->side        = tmp_side;
    pos->castle_perm = tmp_castle_perm;
    pos->en_passant  = tmp_en_passant;

    pos->hash_key = generate_hash_key(pos);
    board_update_lists_material(pos);
    ASSERT(board_check(pos));
}
