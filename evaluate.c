#include "include/definitions.h"
#include "include/validate.h"

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

const int mirror64[64] = {
    56	,	57	,	58	,	59	,	60	,	61	,	62	,	63	,
    48	,	49	,	50	,	51	,	52	,	53	,	54	,	55	,
    40	,	41	,	42	,	43	,	44	,	45	,	46	,	47	,
    32	,	33	,	34	,	35	,	36	,	37	,	38	,	39	,
    24	,	25	,	26	,	27	,	28	,	29	,	30	,	31	,
    16	,	17	,	18	,	19	,	20	,	21	,	22	,	23	,
     8	,	 9	,	10	,	11	,	12	,	13	,	14	,	15	,
     0	,	 1	,	 2	,	 3	,	 4	,	 5	,	 6	,	 7
};

#define MIRROR64(square) (mirror64[(square)])

int evaluatePosition(const board *position) {
    int piece;
    int pieceNumber;
    int square;
    int score = position -> material[WHITE] - position -> material[BLACK];
    
    piece = whitePwn;
    for(pieceNumber = 0; pieceNumber < position -> numberOfPieces[piece]; pieceNumber++) {
        square = position -> pieceList[piece][pieceNumber];
        ASSERT(squareOnBoard(square));
        score += pawnTable[toSQUARE64(square)];
    }

    piece = blackPwn;
    for(pieceNumber = 0; pieceNumber < position -> numberOfPieces[piece]; pieceNumber++) {
        square = position -> pieceList[piece][pieceNumber];
        ASSERT(squareOnBoard(square));
        score -= pawnTable[MIRROR64(toSQUARE64(square))];
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
    } 

    piece = blackRook;
    for(pieceNumber = 0; pieceNumber < position -> numberOfPieces[piece]; pieceNumber++) {
        square = position -> pieceList[piece][pieceNumber];
        ASSERT(squareOnBoard(square));
        score -= rookTable[MIRROR64(toSQUARE64(square))];
    }

    if(position -> boardSide == WHITE) {
        return score;
    }else {
        return -score;
    }
}