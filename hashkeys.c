#include "include/definitions.h"

U64Int generateHashKey(const board *position) {
    int square = 0;
    U64Int finalHashKey = 0;
    int piece = EMPTY_SQR;

    for(square = 0; square < SQUARE_NUMBER; square++) {
        piece = position -> pieces[square];
        if(piece != NULL_SQUARE && piece != EMPTY_SQR && piece != OFFBOARD) {
            ASSERT(piece >= whitePwn && piece <= blackKing);
            finalHashKey ^= pieceKeys[piece][square];
        }
    }

    if(position -> boardSide == WHITE) {
        finalHashKey ^= sideKey;
    }

    if(position -> isEnPassant != NULL_SQUARE) {
        ASSERT(position -> isEnPassant >= 0 && position -> isEnPassant < SQUARE_NUMBER);
        finalHashKey ^= pieceKeys[EMPTY_SQR][position -> isEnPassant];
    }

    ASSERT(position -> castlePermission >= 0 && position -> castlePermission <= 15);
    finalHashKey ^= castleKey[position -> castlePermission];

    return finalHashKey;
}