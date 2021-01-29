#include "include/definitions.h"
#include "include/validate.h"

#define HASH_PIECE(piece, square) (position -> hashKey ^= (pieceKeys[(piece)][(square)]))
#define HASH_CASTLE_PERMISSION (position -> hashKey ^= (castleKey[(position -> castlePermission)]))
#define HASH_BOARD_SIDE (position -> hashKey ^= (sideKey)) 
#define HASH_EN_PASSANT (position -> hashKey ^= (pieceKeys[EMPTY_SQR][(position -> isEnPassant)]))

const int castlePermission[120] = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 13, 15, 15, 15, 12, 15, 15, 14, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15,  7, 15, 15, 15,  3, 15, 15, 11, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
};

static void clearPiece(const int square, board *position) {
    ASSERT(squareOnBoard(square));

    int piece = position -> pieces[square];
    ASSERT(pieceValidate(piece));

    int color = pieceColor[piece];
    int index = 0;
    int tmpPieceNumber = -1;

    HASH_PIECE(piece, square);

    position -> pieces[square] = EMPTY_SQR;
    position -> material[color] -= pieceValue[piece];

    if(pieceBig[piece]) {
        position -> bigPieces[color]--;
        if(pieceMajor[piece]) {
            position -> majorPieces[color]--;
        }else {
            position -> minorPieces[color]--;
        }
    }else {
        CLEARBIT(position -> pawns[color], toSQUARE64(square));
        CLEARBIT(position -> pawns[BOTHCOLORS], toSQUARE64(square));
    }

    for(index = 0; index < position -> numberOfPieces[piece]; index++){ 
        if(position -> pieceList[piece][index] == square) {
            tmpPieceNumber = index;
            break;
        }
    }

    ASSERT(tmpPieceNumber != -1);
    position -> numberOfPieces[piece]--;
    position -> pieceList[piece][tmpPieceNumber] = position -> pieceList[piece][position -> numberOfPieces[piece]];
}

static void addPiece(const int square, board *position, const int piece) {
    ASSERT(pieceValidate(piece));
    ASSERT(squareOnBoard(square));

    int color = pieceColor[piece];

    HASH_PIECE(piece, square);

    position -> pieces[square] = piece;

    if(pieceBig[piece]) {
        position -> bigPieces[color]++;
        if(pieceMajor[piece]) {
            position -> majorPieces[color]++;
        }else {
            position -> minorPieces[color]++;
        }
    }else {
        SETBIT(position -> pawns[color], toSQUARE64(square));
        SETBIT(position -> pawns[BOTHCOLORS], toSQUARE64(square));
    }

    position -> material[color] += pieceValue[piece];
    position -> pieceList[piece][position -> numberOfPieces[piece]++] = square;
}

static void movePiece(const int from, const int to, board *position) {
    ASSERT(squareOnBoard(from));
    ASSERT(squareOnBoard(to));

    int index = 0;
    int piece = position -> pieces[from];
    int color = pieceColor[piece];

    #ifdef DEBUG
        int tmpPieceNumber = FALSE;
    #endif

    HASH_PIECE(piece, from);
    position -> pieces[from] = EMPTY_SQR;

    HASH_PIECE(piece, to);
    position -> pieces[to] = piece;

    if(!pieceBig[piece]) {
        CLEARBIT(position -> pawns[color], toSQUARE64(from));
        CLEARBIT(position -> pawns[BOTHCOLORS], toSQUARE64(from));
        SETBIT(position -> pawns[color], toSQUARE64(to));
        SETBIT(position -> pawns[BOTHCOLORS], toSQUARE64(to));
    }

    for(index = 0; index < position -> numberOfPieces[piece]; index++) {
        if(position -> pieceList[piece][index] == from) {
            position -> pieceList[piece][index] = to;
            #ifdef DEBUG
                tmpPieceNumber = TRUE;
            #endif
            break;
        }
    }

    ASSERT(tmpPieceNumber);
}

int makeMove(board *position, int move) {
    ASSERT(checkBoard(position));

    int from = FROM(move);
    int to = TOSQ(move);
    int side = position -> boardSide;

    // printf("FROM SQUARE: %d\n\n", from);

    ASSERT(squareOnBoard(from));
    ASSERT(squareOnBoard(to));
    ASSERT(boardSideValid(side));
    ASSERT(pieceValidate(position -> pieces[from]));

    position -> history[position -> historyPly].hashKey = position -> hashKey;

    if(move & MOVEFLAG_EnPassant) {
        if(side == WHITE) {
            clearPiece(to - 10, position);
        }else {
            clearPiece(to + 10, position);
        }
    }else if(move & MOVEFLAG_CASTLED) {
        switch(to) {
            case C1: 
                movePiece(A1, D1, position);
                break;
            case C8:
                movePiece(A8, D8, position);
                break;
            case G1:
                movePiece(H1, F1, position);
                break;
            case G8:
                movePiece(H8, F8, position);
                break;
            default: 
                ASSERT(FALSE);
                break;
        }
    }

    if(position -> isEnPassant != NULL_SQUARE) {
        HASH_EN_PASSANT;
    }

    HASH_CASTLE_PERMISSION;

    position -> history[position -> historyPly].move = move;
    position -> history[position -> historyPly].isFiftyMoves = position -> isFiftyMoves;
    position -> history[position -> historyPly].isEnPassant = position -> isEnPassant;
    position -> history[position -> historyPly].castlePermission = position -> castlePermission;

    position -> castlePermission &= castlePermission[from];
    position -> castlePermission &= castlePermission[to];
    position -> isEnPassant = NULL_SQUARE;

    HASH_CASTLE_PERMISSION;

    int captured = CAPTURED(move);
    position -> isFiftyMoves++;

    if(captured != EMPTY_SQR) {
        ASSERT(pieceValidate(captured));
        clearPiece(to, position);
        position -> isFiftyMoves = 0;
    }

    position -> historyPly++;
    position -> ply++;

    if(isPiecePwn[position -> pieces[from]]) {
        position -> isFiftyMoves = 0;
        if(move & MOVEFLAG_PwnStart) {
            if(side == WHITE) {
                position -> isEnPassant = from + 10;
                ASSERT(ranksBoard[position -> isEnPassant] == RANK_3);
            }else {
                position -> isEnPassant = from - 10;
                ASSERT(ranksBoard[position -> isEnPassant] == RANK_6);
            }
            HASH_EN_PASSANT;
        }
    }

    movePiece(from, to, position);

    int promotedPiece = PROMOTED(move);
    if(promotedPiece != EMPTY_SQR) {
        ASSERT(pieceValidate(promotedPiece && !isPiecePwn[promotedPiece]));
        clearPiece(to, position);
        addPiece(to, position, promotedPiece);
    }

    if(isPieceKing[position -> pieces[to]]) {
        position -> kingSquare[position -> boardSide] = to;
    }

    position -> boardSide ^= 1;
    HASH_BOARD_SIDE;

    ASSERT(checkBoard(position));

    if(isSquareAttacked(position -> kingSquare[side], position -> boardSide, position)) {
        takeMove(position);
        return FALSE;
    }

    return TRUE;
}

void takeMove(board *position) {
    ASSERT(checkBoard(position));

    position -> historyPly--;
    position -> ply--;

    int move = position -> history[position -> historyPly].move;
    int from = FROM(move);
    int to = TOSQ(move);

    ASSERT(squareOnBoard(from));
    ASSERT(squareOnBoard(to));

    if(position -> isEnPassant != NULL_SQUARE) {
        HASH_EN_PASSANT;
    }
    HASH_CASTLE_PERMISSION;

    position -> castlePermission = position -> history[position -> historyPly].castlePermission;
    position -> isFiftyMoves = position -> history[position -> historyPly].isFiftyMoves;
    position -> isEnPassant = position -> history[position -> historyPly].isEnPassant;

    if(position -> isEnPassant != NULL_SQUARE) {
        HASH_EN_PASSANT;
    }

    HASH_CASTLE_PERMISSION;

    position -> boardSide ^= 1;
    HASH_BOARD_SIDE;

    if(MOVEFLAG_EnPassant & move) {
        if(position -> boardSide == WHITE) {
            addPiece(to - 10, position, blackPwn);
        }else {
            addPiece(to + 10, position, whitePwn);
        }
    }else if(MOVEFLAG_CASTLED & move) {
        switch(to) {
            case C1: 
                movePiece(D1, A1, position);
                break;
            case C8:
                movePiece(D8, A8, position);
                break;
            case G1: 
                movePiece(F1, H1, position);
                break;
            case G8: 
                movePiece(F8, H8, position);
                break;
            default: 
                ASSERT(FALSE);
                break;
        }
    }

    movePiece(to, from, position);

    if(isPieceKing[position -> pieces[from]]) {
        position -> kingSquare[position -> boardSide] = from;
    }

    int captured = CAPTURED(move);
    if(captured != EMPTY_SQR) {
        ASSERT(pieceValidate(captured));
        addPiece(to, position, captured);
    }

    if(PROMOTED(move) != EMPTY_SQR) {
        ASSERT(pieceValidate(PROMOTED(move)) && !isPiecePwn[PROMOTED(move)]); 
        clearPiece(from, position);
        addPiece(from, position, (pieceColor[PROMOTED(move)] == WHITE ? whitePwn : blackPwn));
    }

    ASSERT(checkBoard(position));
}