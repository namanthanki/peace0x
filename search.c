#include "include/definitions.h"

#define INFINITE 30000
#define MATE 29000

static void checkUp() {
    return;
}

static int isRepetition(const board *position) {
    int index = 0;
    for(index = position -> historyPly - position -> isFiftyMoves; index < position -> historyPly - 1; index++) {
        ASSERT(index >= 0 && index < MAXGAMEMOVES);

        if(position -> hashKey == position -> history[index].hashKey) {
            return TRUE;
        }
    }
    return FALSE;
}

static void clearForSearch(board *position, searchInfo *info) {
    int index = 0;
    int index2 = 0;

    for(index = 0; index < 13; index++) {
        for(index2 = 0; index2 < SQUARE_NUMBER; index2++) {
            position -> searchHistory[index][index2] = 0;
        }
    }

    for(index = 0; index < 2; index++) {
        for(index2 = 0; index2 < MAXDEPTH; index2++) {
            position -> searchKillers[index][index2] = 0;
        }
    }
    clearPvTable(position -> newPvTable);
    position -> ply = 0;

    info -> startTime = getTimeInMiliseconds();
    info -> stopped = 0;
    info -> nodes = 0;
    info -> failHigh = 0;
    info -> failHighFirst = 0;
}

static int quiescene(int alpha, int beta, board *position, searchInfo *info) {
    return 0;
}

static int alphaBeta(int alpha, int beta, int depth, board *position, searchInfo *info, int doNull) {
    ASSERT(checkBoard(position));

    if(depth == 0) {
        info -> nodes++;
        return evaluatePosition(position);
    }

    info -> nodes++;

    if((isRepetition(position) || position -> isFiftyMoves >= 100) && position -> ply) {
        return 0;
    }

    if(position -> ply > MAXDEPTH - 1) {
        return evaluatePosition(position);
    }

    movelist newList[1];
    generateAllMoves(position, newList);

    int moveNumber = 0;
    int legal = 0;
    int oldAlpha = alpha;
    int bestMove = NOMOVE;
    int score = -INFINITE;

    for(moveNumber = 0; moveNumber < newList -> numberOfMoves; moveNumber++) {
        if(!makeMove(position, newList -> moves[moveNumber].move)) {
            continue;
        }

        legal++;
        score = -alphaBeta(-beta, -alpha, depth - 1, position, info, TRUE);
        takeMove(position);

        if(score > alpha) {
            if(score >= beta) {
                if(legal == 1) {
                    info -> failHighFirst++;
                }
                info -> failHigh++;
                return beta;
            }
            alpha = score;
            bestMove = newList -> moves[moveNumber].move;
        }
    }

    if(legal == 0) {
        if(isSquareAttacked(position -> kingSquare[position -> boardSide], position -> boardSide ^ 1, position)) {
            return -MATE + position -> ply;
        }else {
            return 0;
        }
    }

    if(alpha != oldAlpha) {
        storePvMove(position, bestMove);
    }

    return alpha;
}

void searchPosition(board *position, searchInfo *info) {
    int bestMove = NOMOVE;
    int bestScore = -INFINITE;
    int currentDepth = 0;
    int pvMoves = 0;
    int pvNumber = 0;

    clearForSearch(position, info);

    for(currentDepth = 1; currentDepth <= info -> depth; currentDepth++) {
        bestScore = alphaBeta(-INFINITE, INFINITE, currentDepth, position, info, TRUE);
        pvMoves = getPvLine(currentDepth, position);
        bestMove = position -> pvArray[0];

        printf("Depth: %d, Score: %d, Move: %s, Nodes: %ld", currentDepth, bestScore, printMove(bestMove), info -> nodes);

        pvMoves = getPvLine(currentDepth, position);
        printf("Principal Variation");
        for(pvNumber = 0; pvNumber < pvMoves; pvNumber++) {
            printf(" %s", printMove(position -> pvArray[pvNumber]));
        }
        printf("\n");

        printf("Ordering: %.2f\n", (info -> failHighFirst / info -> failHigh));
    }
}