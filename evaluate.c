#include "include/definitions.h"
#include "include/validate.h"

const int pawnIsolated = -10;
const int pawnPassed[8] = { 0, 5, 10, 20, 35, 60, 100, 200 };
const int rookOpenFile = 10;
const int rookSemiOpenFile = 5;
const int queenOpenFile = 5;
const int queenSemiOpenFile = 3;
const int bishopPair = 30;

const int pawnTable[64] = {
    0	,	 0	,	 0	,	  0	,	  0	,	 0	,	 0	,	 0	,
    10	,	10	,	 0	,	-10	,	-10	,	 0	,	10	,	10	,
    5	,	 0	,	 0	,	  5	,	  5	,	 0	,	 0	,	 5	,
    0	,	 0	,	10	,	 20	,	 20	,	10	,	 0	,	 0	,
    5	,	 5	,	 5	,	 10	,	 10	,	 5	,	 5	,	 5	,
    10	,	10	,	10	,	 20	,	 20	,	10	,	10	,	10	,
    20	,	20	,	20	,	 30	,	 30	,	20	,	20	,	20	,
    0	,	0	,	 0	,	  0	,	  0	,	 0	,	 0	,	 0	
};

const int knightTable[64] = {
    0	,	-10	,	 0	,	 0	,	 0	,	 0	,	-10	,	0	,
    0	,	  0	,	 0	,	 5	,	 5	,	 0	,	  0	,	0	,
    0	,	  0	,	10	,	10	,	10	,	10	,	  0	,	0	,
    0	,	  0	,	10	,	20	,	20	,	10	,	  5	,	0	,
    5	,	 10	,	15	,	20	,	20	,	15	,	 10	,	5	,
    5	,	 10	,	10	,	20	,	20	,	10	,	 10	,	5	,
    0	,	  0	,	 5	,	10	,	10	,	 5	,	  0	,	0	,
    0	,	  0	,	 0	,	 0	,	 0	,	 0	,	  0	,	0		
};

const int bishopTable[64] = {
    0	,	 0	,	-10	,	 0	,	 0	,	-10	,	 0	,	0	,
    0	,	 0	,	  0	,	10	,	10	,	  0	,	 0	,	0	,
    0	,	 0	,	 10	,	15	,	15	,	 10	,	 0	,	0	,
    0	,	10	,	 15	,	20	,	20	,	 15	,	10	,	0	,
    0	,	10	,	 15	,	20	,	20	,	 15	,	10	,	0	,
    0	,	 0	,	 10	,	15	,	15	,	 10	,	 0	,	0	,
    0	,	 0	,	  0	,	10	,	10	,	  0	,	 0	,	0	,
    0	,	 0	,	  0	,	 0	,	 0	,	  0	,	 0	,	0	
};

const int rookTable[64] = {
     0	,	 0	,	 5	,	10	,	10	,	 5	,	 0	,	 0	,
     0	,	 0	,	 5	,	10	,	10	,	 5	,	 0	,	 0	,
     0	,	 0	,	 5	,	10	,	10	,	 5	,	 0	,	 0	,
     0	,	 0	,	 5	,	10	,	10	,	 5	,	 0	,	 0	,
     0	,	 0	,	 5	,	10	,	10	,	 5	,	 0	,	 0	,
     0	,	 0	,	 5	,	10	,	10	,	 5	,	 0	,	 0	,
    25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,
     0	,	 0	,	 5	,	10	,	10	,	 5	,	 0	,	 0		
};

const int kingEndgame[64] = {
    -50	,	-10	,	0	,	 0	,	 0	,	 0	,	-10	,	-50	,
    -10 ,	  0	,	10	,	10	,	10	,	10	,	  0	,	-10	,
      0	,	 10	,	15	,	15	,	15	,	15	,	 10	,	  0	,
      0	,	 10	,	15	,	20	,	20	,	15	,	 10	,	  0	,
      0	,	 10	,	15	,   20	,	20	,	15	,	 10	,	  0	,
      0	,	 10	,	15	,	15	,	15	,	15	,	 10	,	  0	,
    -10	,	  0	,	10	,	10	,	10	,	10	,	  0	,	-10	,
    -50	,	-10	,	 0	,	 0 	,	 0	,	 0	,	-10	,	-50
};

const int KingOpening[64] = {
      0	,	  5	,	  5	,	-10	,	-10	,	  0	,	 10	,	  5	,
    -30	,	-30	,	-30	,	-30	,	-30	,	-30	,	-30	,	-30	,
    -50	,	-50	,	-50	,	-50	,	-50	,	-50	,	-50	,	-50	,
    -70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
    -70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
    -70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
    -70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
    -70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70
};

#define ENDGAME_MATERIAL (1 * pieceValue[whiteRook] + 2 * pieceValue[whiteKnight] + 2 * pieceValue[whitePwn] + pieceValue[whiteKing])

int materialDraw(const board *position) {
    if(!position -> numberOfPieces[whiteRook] && !position -> numberOfPieces[blackRook] && !position -> numberOfPieces[whiteQueen] && !position -> numberOfPieces[blackQueen]) {
        if(!position -> numberOfPieces[blackBishop] && !position -> numberOfPieces[whiteBishop]) {
            if(position -> numberOfPieces[whiteKnight] < 3 && position -> numberOfPieces[blackKnight] < 3) {
                return TRUE;
            }else if(!position -> numberOfPieces[whiteKnight] && !position -> numberOfPieces[blackKnight]) {
                if(abs(position -> numberOfPieces[whiteBishop] - position -> numberOfPieces[blackBishop]) < 2) {
                    return TRUE;
                }
            }else if((position -> numberOfPieces[whiteKnight] < 3 && !position -> numberOfPieces[whiteBishop]) || (position -> numberOfPieces[whiteBishop] == 1 && !position -> numberOfPieces[whiteKnight])) {
                if((position -> numberOfPieces[blackKnight] < 3 && !position -> numberOfPieces[blackBishop]) || (position -> numberOfPieces[blackBishop] == 1 && !position -> numberOfPieces[blackKnight])) {
                    return TRUE;
                }
            }
        }
    }else if(!position -> numberOfPieces[whiteQueen] && !position -> numberOfPieces[blackQueen]) {
        if(position -> numberOfPieces[whiteRook] == 1 && position -> numberOfPieces[blackRook] == 1) {
            if((position -> numberOfPieces[whiteKnight] + position -> numberOfPieces[blackBishop]) < 2 && (position -> numberOfPieces[blackKnight] + position -> numberOfPieces[blackBishop]) < 2) {
                return TRUE;
            }
        }else if(position -> numberOfPieces[whiteRook] == 1 && !position -> numberOfPieces[blackRook] == 1) {
            if((position -> numberOfPieces[whiteKnight] + position -> numberOfPieces[whiteBishop] == 0) && (((position -> numberOfPieces[blackKnight] + position -> numberOfPieces[blackBishop]) == 1) || ((position -> numberOfPieces[blackKnight] + position -> numberOfPieces[blackBishop]) == 2))) {
                return TRUE;
            }
        }else if(position->numberOfPieces[blackRook] == 1 && !position->numberOfPieces[whiteRook] == 1) {
            if((position -> numberOfPieces[blackKnight] + position -> numberOfPieces[blackBishop] == 0) && (((position -> numberOfPieces[whiteKnight] + position -> numberOfPieces[whiteBishop]) == 1) || ((position -> numberOfPieces[whiteKnight] + position -> numberOfPieces[whiteBishop]) == 2))) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

int evaluatePosition(const board *position) {
    int piece;
    int pieceNumber;
    int square;
    int score = position -> material[WHITE] - position -> material[BLACK];

    if(!position -> numberOfPieces[whitePwn] && !position -> numberOfPieces[blackPwn] && materialDraw(position) == TRUE) {
        return 0;
    }
    
    piece = whitePwn;
    for(pieceNumber = 0; pieceNumber < position -> numberOfPieces[piece]; pieceNumber++) {
        square = position -> pieceList[piece][pieceNumber];
        ASSERT(squareOnBoard(square));
        score += pawnTable[toSQUARE64(square)];

        if((isolatedPwnMask[toSQUARE64(square)] & position -> pawns[WHITE]) == 0)  {
            score += pawnIsolated;
        }

        if ((whitePassedMask[toSQUARE64(square)] & position->pawns[BLACK]) == 0) {
            score += pawnPassed[ranksBoard[square]];
        }
    }

    piece = blackPwn;
    for(pieceNumber = 0; pieceNumber < position -> numberOfPieces[piece]; pieceNumber++) {
        square = position -> pieceList[piece][pieceNumber];
        ASSERT(squareOnBoard(square));
        score -= pawnTable[MIRROR64(toSQUARE64(square))];

        if ((isolatedPwnMask[toSQUARE64(square)] & position->pawns[BLACK]) == 0) {
            score -= pawnIsolated;
        }

        if((blackPassedMask[toSQUARE64(square)] & position -> pawns[WHITE]) == 0) {
            score -= pawnPassed[7 - ranksBoard[square]];
        }
    }

    piece = whiteKnight;
    for(pieceNumber = 0; pieceNumber < position -> numberOfPieces[piece]; pieceNumber++) {
        square = position -> pieceList[piece][pieceNumber];
        ASSERT(squareOnBoard(square));
        score += knightTable[toSQUARE64(square)];
    } 

    piece = blackKnight;
    for(pieceNumber = 0; pieceNumber < position -> numberOfPieces[piece]; pieceNumber++) {
        square = position -> pieceList[piece][pieceNumber];
        ASSERT(squareOnBoard(square));
        score -= knightTable[MIRROR64(toSQUARE64(square))];
    }

    piece = whiteBishop;
    for(pieceNumber = 0; pieceNumber < position -> numberOfPieces[piece]; pieceNumber++) {
        square = position -> pieceList[piece][pieceNumber];
        ASSERT(squareOnBoard(square));
        score += bishopTable[toSQUARE64(square)];
    } 

    piece = blackBishop;
    for(pieceNumber = 0; pieceNumber < position -> numberOfPieces[piece]; pieceNumber++) {
        square = position -> pieceList[piece][pieceNumber];
        ASSERT(squareOnBoard(square));
        score -= bishopTable[MIRROR64(toSQUARE64(square))];
    }

    piece = whiteRook;
    for(pieceNumber = 0; pieceNumber < position -> numberOfPieces[piece]; pieceNumber++) {
        square = position -> pieceList[piece][pieceNumber];
        ASSERT(squareOnBoard(square));
        score += rookTable[toSQUARE64(square)];

        if(!(position -> pawns[BOTHCOLORS] & fileBitBoardMask[filesBoard[square]])) {
            score += rookOpenFile;
        }else if(!(position -> pawns[WHITE] & fileBitBoardMask[filesBoard[square]])) {
            score += rookSemiOpenFile;
        }
    } 

    piece = blackRook;
    for(pieceNumber = 0; pieceNumber < position -> numberOfPieces[piece]; pieceNumber++) {
        square = position -> pieceList[piece][pieceNumber];
        ASSERT(squareOnBoard(square));
        score -= rookTable[MIRROR64(toSQUARE64(square))];

        if (!(position->pawns[BOTHCOLORS] & fileBitBoardMask[filesBoard[square]])) {
            score -= rookOpenFile;
        }
        else if (!(position->pawns[BLACK] & fileBitBoardMask[filesBoard[square]])) {
            score -= rookSemiOpenFile;
        }
    }

    piece = whiteQueen;
    for(pieceNumber = 0; pieceNumber < position -> numberOfPieces[piece]; pieceNumber++) {
        square = position -> pieceList[piece][pieceNumber];
        ASSERT(squareOnBoard(position));

        if (!(position->pawns[BOTHCOLORS] & fileBitBoardMask[filesBoard[square]])) {
            score += queenOpenFile;
        }
        else if (!(position->pawns[WHITE] & fileBitBoardMask[filesBoard[square]])) {
            score += queenSemiOpenFile;
        }
    }

    piece = blackQueen;
    for(pieceNumber = 0; pieceNumber < position -> numberOfPieces[piece]; pieceNumber++) {
        square = position -> pieceList[piece][pieceNumber];
        ASSERT(squareOnBoard(position));

        if (!(position->pawns[BOTHCOLORS] & fileBitBoardMask[filesBoard[square]])) {
            score -= queenOpenFile;
        }
        else if (!(position->pawns[BLACK] & fileBitBoardMask[filesBoard[square]])) {
            score -= queenSemiOpenFile;
        }
    }

    piece = whiteKing;
    square = position -> pieceList[piece][0];
    if(position -> material[BLACK] <= ENDGAME_MATERIAL) {
        score += kingEndgame[toSQUARE64(square)];
    }else {
        score += KingOpening[toSQUARE64(square)];
    }

    piece = blackKing;
    square = position -> pieceList[piece][0];
    if (position -> material[WHITE] <= ENDGAME_MATERIAL) {
        score -= kingEndgame[MIRROR64(toSQUARE64(square))];
    }
    else {
        score -= KingOpening[MIRROR64(toSQUARE64(square))];
    }
    
    if(position -> numberOfPieces[whiteBishop] >= 2) {
        score += bishopPair;
    }
    if(position -> numberOfPieces[blackBishop] >= 2) {
        score -= bishopPair;
    }

    if(position -> boardSide == WHITE) {
        return score;
    }else {
        return -score;
    }
}