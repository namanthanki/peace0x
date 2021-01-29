#include "include/definitions.h"
#include "include/validate.h"

#define MOVE(fr, to, capd, prom, flrnk) ((fr) | ((to) << 7) | ((capd) << 14) | ((prom) << 20) | (flrnk))
#define squareOffboard(square) (filesBoard[(square)] == OFFBOARD) 

const int loopSlidePieces[8] = {
    whiteBishop, whiteRook, whiteQueen, 0, blackBishop, blackRook, blackQueen, 0
};

const int loopNonSlidePieces[6] = {
    whiteKnight, whiteKing, 0, blackKnight, blackKing, 0
};

const int loopSlideIndex[2] = { 0, 4 };
const int loopNonSlideIndex[2] = { 0, 3 };

const int pieceDirection[13][8] = {
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { -8, -19, -21, -12, 8, 19, 21, 12 },
    { -9, -11, 11, 9, 0, 0, 0, 0 },
    { -1, -10, 1, 10, 0, 0, 0, 0 },
    { -1, -10, 1, 10, -9, -11, 11, 9 },
    { -1, -10, 1, 10, -9, -11, 11, 9 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { -8, -19, -21, -12, 8, 19, 21, 12 },
    { -9, -11, 11, 9, 0, 0, 0, 0 },
    { -1, -10, 1, 10, 0, 0, 0, 0 },
    { -1, -10, 1, 10, -9, -11, 11, 9 },
    { -1, -10, 1, 10, -9, -11, 11, 9 }
};

const int numberOfDirections[13] = {
    0, 0, 8, 4, 4, 8, 8, 0, 8, 4, 4, 8, 8
};

int moveExists(board *position, const int move) {
    movelist newList[1];
    generateAllMoves(position, newList);
    
    int moveNumber = 0;
    for(moveNumber = 0; moveNumber < newList -> numberOfMoves; moveNumber++) {
        if(!makeMove(position, newList -> moves[moveNumber].move)) {
            continue;
        }
        takeMove(position);
        if(newList -> moves[moveNumber].move == move) {
            return TRUE;
        }
    }
    return FALSE;
}

static void addQuietMove(const board *position, int move, movelist *list) {
    list -> moves[list -> numberOfMoves].move = move;
    list -> moves[list -> numberOfMoves].score = 0;
    list -> numberOfMoves++;
}

static void addCapturedMove(const board *position, int move, movelist *list) {
    list -> moves[list -> numberOfMoves].move = move;
    list -> moves[list -> numberOfMoves].score = 0;
    list -> numberOfMoves++;
}

static void addEnPassantMove(const board *position, int move, movelist *list) {
    list -> moves[list -> numberOfMoves].move = move;
    list -> moves[list -> numberOfMoves].score = 0;
    list -> numberOfMoves++;
}

static void addWhitePwnCaptureMove(const board *position, const int from, const int to, const int captured, movelist *list) {
    ASSERT(pieceValidateEmpty(captured));
    ASSERT(squareOnBoard(from));
    ASSERT(squareOnBoard(to));

    if(ranksBoard[from] == RANK_7) {
        addCapturedMove(position, MOVE(from, to, captured, whiteQueen, 0), list);
        addCapturedMove(position, MOVE(from, to, captured, whiteRook, 0), list);
        addCapturedMove(position, MOVE(from, to, captured, whiteBishop, 0), list);
        addCapturedMove(position, MOVE(from, to, captured, whiteKnight, 0), list);
    }else {
        addCapturedMove(position, MOVE(from, to, captured, EMPTY_SQR, 0), list);
    }
}

static void addWhitePwnMove(const board *position, const int from, const int to, movelist *list) {
    ASSERT(squareOnBoard(from));
    ASSERT(squareOnBoard(to));

    if(ranksBoard[from] == RANK_7) {
        addCapturedMove(position, MOVE(from, to, EMPTY_SQR, whiteQueen, 0), list);
        addCapturedMove(position, MOVE(from, to, EMPTY_SQR, whiteRook, 0), list);
        addCapturedMove(position, MOVE(from, to, EMPTY_SQR, whiteBishop, 0), list);
        addCapturedMove(position, MOVE(from, to, EMPTY_SQR, whiteKnight, 0), list);
    }else {
        addCapturedMove(position, MOVE(from, to, EMPTY_SQR, EMPTY_SQR, 0), list);
    }
}

static void addBlackPwnCaptureMove(const board *position, const int from, const int to, const int captured, movelist *list) {
    ASSERT(pieceValidateEmpty(captured));
    ASSERT(squareOnBoard(from));
    ASSERT(squareOnBoard(to));

    if(ranksBoard[from] == RANK_2) {
        addCapturedMove(position, MOVE(from, to, captured, blackQueen, 0), list);
        addCapturedMove(position, MOVE(from, to, captured, blackRook, 0), list);
        addCapturedMove(position, MOVE(from, to, captured, blackBishop, 0), list);
        addCapturedMove(position, MOVE(from, to, captured, blackKnight, 0), list);
    }else {
        addCapturedMove(position, MOVE(from, to, captured, EMPTY_SQR, 0), list);
    }
}

static void addBlackPwnMove(const board *position, const int from, const int to, movelist *list) {
    ASSERT(squareOnBoard(from));
    ASSERT(squareOnBoard(to));

    if(ranksBoard[from] == RANK_2) {
        addCapturedMove(position, MOVE(from, to, EMPTY_SQR, blackQueen, 0), list);
        addCapturedMove(position, MOVE(from, to, EMPTY_SQR, blackRook, 0), list);
        addCapturedMove(position, MOVE(from, to, EMPTY_SQR, blackBishop, 0), list);
        addCapturedMove(position, MOVE(from, to, EMPTY_SQR, blackKnight, 0), list);
    }else {
        addCapturedMove(position, MOVE(from, to, EMPTY_SQR, EMPTY_SQR, 0), list);
    }
}

void generateAllMoves(const board *position, movelist *list) {
    ASSERT(checkBoard(position));
    
    list -> numberOfMoves = 0;

    int piece = EMPTY_SQR;
    int boardSide = position -> boardSide;
    int square = 0;
    int targetSquare = 0;
    int pieceNumber = 0;
    int direction = 0;
    int index = 0;
    int pieceIndex = 0;

    if(boardSide == WHITE) {
        for(pieceNumber = 0; pieceNumber < position -> numberOfPieces[whitePwn]; pieceNumber++) {
            square = position -> pieceList[whitePwn][pieceNumber];
            ASSERT(squareOnBoard(square));

            if(position -> pieces[square + 10] == EMPTY_SQR) {
                addWhitePwnMove(position, square, square + 10, list);
                if(ranksBoard[square] == RANK_2 && position -> pieces[square + 20] == EMPTY_SQR) {
                    addQuietMove(position, MOVE(square, (square + 20), EMPTY_SQR, EMPTY_SQR, MOVEFLAG_PwnStart), list);
                }
            }

            if(!squareOffboard(square + 9) && pieceColor[position -> pieces[square + 9]] == BLACK) {
                addWhitePwnCaptureMove(position, square, square + 9, position -> pieces[square + 9], list);
            }
            if(!squareOffboard(square + 11) && pieceColor[position -> pieces[square + 11]] == BLACK) {
                addWhitePwnCaptureMove(position, square, square + 11, position -> pieces[square + 11], list);
            }

            if(square + 9 == position -> isEnPassant) {
                addCapturedMove(position, MOVE(square, square + 9, EMPTY_SQR, EMPTY_SQR, MOVEFLAG_EnPassant), list);
            }
            if(square + 11 == position -> isEnPassant) {
                addCapturedMove(position, MOVE(square, square + 11, EMPTY_SQR, EMPTY_SQR, MOVEFLAG_EnPassant), list);
            }
        }

        if(position -> castlePermission & wKingSideCastle) {
            if(position -> pieces[F1] == EMPTY_SQR && position -> pieces[G1] == EMPTY_SQR) {
                if(!isSquareAttacked(E1, BLACK, position) && !isSquareAttacked(F1, BLACK, position)) {
                    addQuietMove(position, MOVE(E1, G1, EMPTY_SQR, EMPTY_SQR, MOVEFLAG_CASTLED), list);
                }
            }
        }

        if(position -> castlePermission & wQueenSideCastle) {
            if(position -> pieces[D1] == EMPTY_SQR && position -> pieces[C1] == EMPTY_SQR && position -> pieces[B1] == EMPTY_SQR) {
                if(!isSquareAttacked(E1, BLACK, position) && !isSquareAttacked(D1, BLACK, position)) {
                    addQuietMove(position, MOVE(E1, C1, EMPTY_SQR, EMPTY_SQR, MOVEFLAG_CASTLED), list);
                }
            }
        }

    }else {
        for(pieceNumber = 0; pieceNumber < position -> numberOfPieces[blackPwn]; pieceNumber++) {
            square = position -> pieceList[blackPwn][pieceNumber];
            ASSERT(squareOnBoard(square));

            if(position -> pieces[square - 10] == EMPTY_SQR) {
                addBlackPwnMove(position, square, square - 10, list);
                if(ranksBoard[square] == RANK_7 && position -> pieces[square - 20] == EMPTY_SQR) {
                    addQuietMove(position, MOVE(square, (square - 20), EMPTY_SQR, EMPTY_SQR, MOVEFLAG_PwnStart), list);
                }
            }

            if(!squareOffboard(square - 9) && pieceColor[position -> pieces[square - 9]] == WHITE) {
                addBlackPwnCaptureMove(position, square, square - 9, position -> pieces[square - 9], list);
            }
            if(!squareOffboard(square - 11) && pieceColor[position -> pieces[square - 11]] == WHITE) {
                addBlackPwnCaptureMove(position, square, square - 11, position -> pieces[square - 11], list);
            }

            if(square - 9 == position -> isEnPassant) {
                addCapturedMove(position, MOVE(square, square - 9, EMPTY_SQR, EMPTY_SQR, MOVEFLAG_EnPassant), list);
            }
            if(square - 11 == position -> isEnPassant) {
                addCapturedMove(position, MOVE(square, square - 11, EMPTY_SQR, EMPTY_SQR, MOVEFLAG_EnPassant), list);
            }
        }

        if(position -> castlePermission & bKingSideCastle) {
            if(position -> pieces[F8] == EMPTY_SQR && position -> pieces[G8] == EMPTY_SQR) {
                if(!isSquareAttacked(E8, WHITE, position) && !isSquareAttacked(F8, WHITE, position)) {
                    addQuietMove(position, MOVE(E8, G8, EMPTY_SQR, EMPTY_SQR, MOVEFLAG_CASTLED), list);
                }
            }
        }

        if(position -> castlePermission & bQueenSideCastle) {
            if(position -> pieces[D8] == EMPTY_SQR && position -> pieces[C8] == EMPTY_SQR && position -> pieces[B8] == EMPTY_SQR) {
                if(!isSquareAttacked(E8, WHITE, position) && !isSquareAttacked(D8, WHITE, position)) {
                    addQuietMove(position, MOVE(E8, C8, EMPTY_SQR, EMPTY_SQR, MOVEFLAG_CASTLED), list);
                }
            }
        }

    }

    pieceIndex = loopSlideIndex[boardSide];
    piece = loopSlidePieces[pieceIndex++];

    while(piece != 0) {
        ASSERT(pieceValidate(piece));
        
        for(pieceNumber = 0; pieceNumber < position -> numberOfPieces[piece]; pieceNumber++) {
            square = position -> pieceList[piece][pieceNumber];
            ASSERT(squareOnBoard(square));

            for(index = 0; index < numberOfDirections[piece]; index++) {
                direction = pieceDirection[piece][index];
                targetSquare = square + direction;

                while(!squareOffboard(targetSquare)) {
                    if(position -> pieces[targetSquare] != EMPTY_SQR) {
                        if(pieceColor[position -> pieces[targetSquare]] == boardSide ^ 1) {
                            addCapturedMove(position, MOVE(square, targetSquare, position -> pieces[targetSquare], EMPTY_SQR, 0), list);
                        }
                        break;
                    }
                    addQuietMove(position, MOVE(square, targetSquare, EMPTY_SQR, EMPTY_SQR, 0), list);
                    targetSquare += direction;
                }
            }
        }
        
        piece = loopSlidePieces[pieceIndex++];
    }

    pieceIndex = loopNonSlideIndex[boardSide];
    piece = loopNonSlidePieces[pieceIndex++];

    while(piece != 0) {
        ASSERT(pieceValidate(piece));

        for(pieceNumber = 0; pieceNumber < position -> numberOfPieces[piece]; pieceNumber++) {
            square = position -> pieceList[piece][pieceNumber];
            ASSERT(squareOnBoard(square));

            for(index = 0; index < numberOfDirections[piece]; index++) {
                direction = pieceDirection[piece][index];
                targetSquare = square + direction;

                if(squareOffboard(targetSquare)) {
                    continue;
                }

                if(position -> pieces[targetSquare] != EMPTY_SQR) {
                    if(pieceColor[position -> pieces[targetSquare]] == boardSide ^ 1) {
                        addCapturedMove(position, MOVE(square, targetSquare, position -> pieces[targetSquare], EMPTY_SQR, 0), list);
                    }
                    continue;
                }
                addQuietMove(position, MOVE(square, targetSquare, EMPTY_SQR, EMPTY_SQR, 0), list);
            }
        }

        piece = loopNonSlidePieces[pieceIndex++];
    }

}