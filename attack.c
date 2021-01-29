#include "include/definitions.h"
#include "include/validate.h"

const int knightDirection[8] = { -8, -19, -21, -12, 8, 19, 21, 12 };
const int rookDirection[4] = { -1, -10, 1, 10 };
const int bishopDirection[4] = { -9, -11, 11, 9 };
const int kingDirection[8] = { -1, -10, 1, 10, -9, -11, 11, 9 };

int isSquareAttacked(const int square, const int boardSide, const board *position) {
    int piece;
    int index;
    int tmpSquare;
    int direction;

    ASSERT(squareOnBoard(square));
    ASSERT(boardSideValid(boardSide));
    ASSERT(checkBoard(position));

    if(boardSide == WHITE) {
        if(position -> pieces[square - 11] == whitePwn || position -> pieces[square - 9] == whitePwn) {
            return TRUE;
        }
    }else {
        if(position -> pieces[square + 11] == blackPwn || position -> pieces[square + 9] == blackPwn) {
            return TRUE;
        }
    }

    for(index = 0; index < 8; index++) {
        piece = position -> pieces[square + knightDirection[index]];
        if(piece != OFFBOARD && isKn(piece) && pieceColor[piece]  == boardSide) {
            return TRUE;
        }
    }

    for(index = 0; index < 4; index++) {
        direction = rookDirection[index];
        tmpSquare = square + direction;
        piece = position -> pieces[tmpSquare];
        
        while(piece != OFFBOARD) {
            if(piece != EMPTY_SQR) {
                if(isRQ(piece) && pieceColor[piece] == boardSide) {
                    return TRUE;
                }
                break;
            }
            tmpSquare += direction;
            piece = position -> pieces[tmpSquare];
        }
    }

    for(index = 0; index < 4; index++) {
        direction = bishopDirection[index];
        tmpSquare = square + direction;
        piece = position -> pieces[tmpSquare];
        
        while(piece != OFFBOARD) {
            if(piece != EMPTY_SQR) {
                if(isBQ(piece) && pieceColor[piece] == boardSide) {
                    return TRUE;
                }
                break;
            }
            tmpSquare += direction;
            piece = position -> pieces[tmpSquare];
        }
    }

    for(index = 0; index < 8; index++) {
        piece = position -> pieces[square + kingDirection[index]];
        if(piece != OFFBOARD && isKi(piece) && pieceColor[piece] == boardSide) {
            return TRUE;
        }
    }

    return FALSE;
}