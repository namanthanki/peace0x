#include "definitions.h" 

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