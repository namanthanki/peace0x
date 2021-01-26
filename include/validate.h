#ifndef VALIDATE_H
#define VALIDATE_H

#include <stdbool.h>

// static inline bool squareOnBoard(const int square) {

// }
// extern int boardSideValid(const int boardSide);
// extern int fileRankValid(const int fileRank);
// extern int pieceValidateEmpty(const int piece);
// extern int pieceValidate(const int piece);


#include "definitions.h"

static inline bool squareOnBoard(const int square) {
    // return filesBoard[square] == OFFBOARD ? FALSE : TRUE;
	return filesBoard[square] != OFFBOARD;
}

static inline bool boardSideValid(const int boardSide) {
    return (boardSide == WHITE || boardSide == BLACK);
}

static inline bool fileRankValid(const int fileRank) {
    return (fileRank >= 0 && fileRank <= 7);
}

static inline bool pieceValidateEmpty(const int piece) {
    return (piece >= EMPTY_SQR && piece <= blackKing);
}

static inline bool pieceValidate(const int piece) {
	// #ifdef DEBUG
	// 	fprintf(stderr, "piece: %d\n", piece);
	// #endif
    return (piece >= whitePwn && piece <= blackKing);
}



#endif /* VALIDATE_H */