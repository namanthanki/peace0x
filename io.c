#include "definitions.h" 
#include "validate.h"

char *printSquareString(const int square) {
    static char squareString[3];
    int file =  filesBoard[square];
    int rank = ranksBoard[square];

    sprintf(squareString,"%c%c", ('a' + file), ('1' + rank));
    return squareString;
}

char *printMove(const int move) {
    static char moveString[6];

    int fileFrom = filesBoard[FROM(move)];
    int rankForm = ranksBoard[FROM(move)];
    int fileTo = filesBoard[TOSQ(move)];
    int rankTo = ranksBoard[TOSQ(move)];

    int promoted = PROMOTED(move);

    if(promoted) {
        char pieceChar = 'q';
        if(isKn(promoted)) {
            pieceChar = 'n';
        }else if(isRQ(promoted) && !isBQ(promoted)) {
            pieceChar = 'r';
        }else if(!isRQ(promoted) && isBQ(promoted)) {
            pieceChar = 'b';
        }
        sprintf(moveString, "%c%c%c%c%c", ('a' + fileFrom), ('1' + rankForm), ('a' + fileTo), ('1' + rankTo), pieceChar);
    }else {
        sprintf(moveString, "%c%c%c%c", ('a' + fileFrom), ('1' + rankForm), ('a' + fileTo), ('1' + rankTo));
    }

    return moveString;
}

int parseMove(char *ptrChar, board *position) {
    if(ptrChar[1] > '8' || ptrChar[1] < '1') {
        return NOMOVE;
    }
    if(ptrChar[3] > '8' || ptrChar[3] < '1') {
        return NOMOVE;
    }
    if(ptrChar[0] > 'h' || ptrChar[0] < 'a') {
        return NOMOVE;
    }
    if(ptrChar[2] > 'h' || ptrChar[2] < 'a') {
        return NOMOVE;
    }

    int from = FILE_RANK_TO_SQUARE(ptrChar[0] - 'a', ptrChar[1] - '1');
    int to = FILE_RANK_TO_SQUARE(ptrChar[2] - 'a', ptrChar[3] - '1');

    ASSERT(squareOnBoard(from) && squareOnBoard(to));

    movelist newList[1];
    generateAllMoves(position, newList);
    int moveNumber = 0;
    int move = NOMOVE;
    int promotedPiece = EMPTY_SQR;
    
    for(moveNumber = 0; moveNumber < newList -> numberOfMoves; moveNumber++) {
        move = newList -> moves[moveNumber].move;
        if(FROM(move) == from && TOSQ(move) == to) {
            promotedPiece = PROMOTED(move);
            if(promotedPiece != EMPTY_SQR) {
                if(isRQ(promotedPiece) && !isBQ(promotedPiece) && ptrChar[4] == 'r') {
                    return move;
                }else if(!isRQ(promotedPiece) && isBQ(promotedPiece) && ptrChar[4] == 'b') {
                    return move;
                }else if(isRQ(promotedPiece) && isBQ(promotedPiece) && ptrChar[4] == 'q') {
                    return move;
                }else if(isKn(promotedPiece) && ptrChar[4] == 'n') {
                    return move;
                }
                continue;
            }
            return move;
        }
    }
    return NOMOVE;
}

void printMoveList(const movelist *list) {
    int index = 0;
    int score = 0;
    int move = 0;
    printf("MoveList: %d\n", list -> numberOfMoves);

    for(index = 0; index < list -> numberOfMoves; index++) {
        move = list -> moves[index].move;
        score = list -> moves[index].score;
        printf("Move: %d > %s (score: %d)\n", index + 1, printMove(move), score);
    }
    printf("Move List Total %d Moves: \n\n", list -> numberOfMoves);
}