#include "peace0x/attack.h"
#include "peace0x/board.h"

static const int knight_dir[8] = { -8, -19, -21, -12, 8, 19, 21, 12 };
static const int rook_dir[4]   = { -1, -10, 1, 10 };
static const int bishop_dir[4] = { -9, -11, 11, 9 };
static const int king_dir[8]   = { -1, -10, 1, 10, -9, -11, 11, 9 };

bool is_square_attacked(Square square, Color side, const Board *pos) {
    ASSERT(square_on_board(square));
    ASSERT(side_valid(side));
    ASSERT(pos != NULL);

    /* Pawns */
    if (side == COLOR_WHITE) {
        if (pos->pieces[square - 11] == PIECE_W_PAWN || pos->pieces[square - 9] == PIECE_W_PAWN) {
            return true;
        }
    } else {
        if (pos->pieces[square + 11] == PIECE_B_PAWN || pos->pieces[square + 9] == PIECE_B_PAWN) {
            return true;
        }
    }

    /* Knights */
    for (int i = 0; i < 8; i++) {
        Piece piece = pos->pieces[square + knight_dir[i]];
        if (piece != (Piece)SQ_OFFBOARD && piece_knight[piece] && piece_color[piece] == side) {
            return true;
        }
    }

    /* Rooks & Queens */
    for (int i = 0; i < 4; i++) {
        int dir = rook_dir[i];
        Square tmp_sq = (Square)(square + dir);
        Piece piece = pos->pieces[tmp_sq];

        while (piece != (Piece)SQ_OFFBOARD) {
            if (piece != PIECE_EMPTY) {
                if (piece_rook_queen[piece] && piece_color[piece] == side) {
                    return true;
                }
                break;
            }
            tmp_sq = (Square)(tmp_sq + dir);
            piece = pos->pieces[tmp_sq];
        }
    }

    /* Bishops & Queens */
    for (int i = 0; i < 4; i++) {
        int dir = bishop_dir[i];
        Square tmp_sq = (Square)(square + dir);
        Piece piece = pos->pieces[tmp_sq];

        while (piece != (Piece)SQ_OFFBOARD) {
            if (piece != PIECE_EMPTY) {
                if (piece_bishop_queen[piece] && piece_color[piece] == side) {
                    return true;
                }
                break;
            }
            tmp_sq = (Square)(tmp_sq + dir);
            piece = pos->pieces[tmp_sq];
        }
    }

    /* Kings */
    for (int i = 0; i < 8; i++) {
        Piece piece = pos->pieces[square + king_dir[i]];
        if (piece != (Piece)SQ_OFFBOARD && piece_king[piece] && piece_color[piece] == side) {
            return true;
        }
    }

    return false;
}
