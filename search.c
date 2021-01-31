#include "include/definitions.h"
#define MATE 29000

static void checkUp(searchInfo *info) {
    if(info -> timeSet == TRUE && getTimeInMiliseconds() > info -> stopTime) {
        info -> stopped = TRUE;
    }

    readInput(info);
}

static void pickNextMove(int moveNumber, movelist* newList) {
    s_move temp;
    int index = 0;
    int bestScore = 0;
    int bestNumber = moveNumber;

    for (index = moveNumber; index < newList -> numberOfMoves; index++) {
        if (newList -> moves[index].score > bestScore) {
            bestScore = newList -> moves[index].score;
            bestNumber = index;
        }
    }

    temp = newList -> moves[moveNumber];
    newList -> moves[moveNumber] = newList -> moves[bestNumber];
    newList -> moves[bestNumber] = temp;
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
    
    position -> hashTable -> overWrite = 0;
    position -> hashTable -> hit = 0;
    position -> hashTable -> cut = 0;
    position -> ply = 0;

    info -> stopped = 0;
    info -> nodes = 0;
    info -> failHigh = 0;
    info -> failHighFirst = 0;
}

static int quiescence(int alpha, int beta, board *position, searchInfo *info) {
    ASSERT(checkBoard(position));
    
    if((info -> nodes & 2047) == 0) { 
        checkUp(info);
    }

    info -> nodes++;

    if((isRepetition(position) || position -> isFiftyMoves >= 100) && position -> ply) {
        return 0;
    }

    if(position -> ply > MAXDEPTH - 1) {
        return evaluatePosition(position);
    }

    int score = evaluatePosition(position);

    if(score >= beta) {
        return beta;
    }

    if(score > alpha) {
        alpha = score;
    }

    movelist newList[1];
    generateAllCaptures(position, newList);

    int moveNumber = 0;
    int legal = 0;
    int oldAlpha = alpha;
    int bestMove = NOMOVE;
    score = -INFINITE;

    for (moveNumber = 0; moveNumber < newList->numberOfMoves; moveNumber++) {
        pickNextMove(moveNumber, newList);

        if (!makeMove(position, newList->moves[moveNumber].move)) {
            continue;
        }

        legal++;
        score = -quiescence(-beta, -alpha, position, info);
        takeMove(position);

        if(info -> stopped == TRUE) { 
            return 0;
        }

        if (score > alpha) {
            if (score >= beta) {
                if (legal == 1) {
                    info->failHighFirst++;
                }
                info->failHigh++;
                return beta;
            }
            alpha = score;
            bestMove = newList->moves[moveNumber].move;
        }
    }
    return alpha;
}

static int alphaBeta(int alpha, int beta, int depth, board *position, searchInfo *info, int doNull) {
    ASSERT(checkBoard(position));

    if(depth == 0) {
        info -> nodes++;
        return quiescence(alpha, beta, position, info);
        // return evaluatePosition(position);
    }

    if((info -> nodes & 2047) == 0) {
        checkUp(info);
    }

    info -> nodes++;

    if((isRepetition(position) || position -> isFiftyMoves >= 100) && position -> ply) {
        return 0;
    }

    if(position -> ply > MAXDEPTH - 1) {
        return evaluatePosition(position);
    }

    int inCheck = isSquareAttacked(position -> kingSquare[position -> boardSide], position -> boardSide ^ 1, position);
    if (inCheck == TRUE) {
        depth++;
    }

    int score = -INFINITE;
    int pvMove = NOMOVE;

    if(probeHashEntry(position, &pvMove, &score, alpha, beta, depth) == TRUE) {
        position -> hashTable -> cut++;
        return score;
    }

    if(doNull && !inCheck && position -> ply && (position -> bigPieces[position -> boardSide] >= 2) && depth >= 4) {
        makeNullMove(position);
        score = -alphaBeta(-beta, -beta + 1, depth - 4, position, info, FALSE);
        takeNullMove(position);
        if(info -> stopped == TRUE) {
            return 0;
        }
        if(score >= beta && abs(score) < ISMATE) {
            return beta;
        }
    }


    movelist newList[1];
    generateAllMoves(position, newList);

    int moveNumber = 0;
    int legal = 0;
    int oldAlpha = alpha;
    int bestMove = NOMOVE;
    int bestScore = -INFINITE;
    score = -INFINITE;

    if (pvMove != NOMOVE) {
        for(moveNumber = 0; moveNumber < newList -> numberOfMoves; moveNumber++) {
            if (newList -> moves[moveNumber].move == pvMove) {
                newList -> moves[moveNumber].score = 2000000;
                break;
            }
        }
    }

    for(moveNumber = 0; moveNumber < newList -> numberOfMoves; moveNumber++) {
        pickNextMove(moveNumber, newList);

        if(!makeMove(position, newList -> moves[moveNumber].move)) {
            continue;
        }

        legal++;
        score = -alphaBeta(-beta, -alpha, depth - 1, position, info, TRUE);
        takeMove(position);

        if (info->stopped == TRUE) {
            return 0;
        }
        if(score > bestScore) {
            bestScore = score;
            bestMove = newList -> moves[moveNumber].move;
            if (score > alpha) {
                if (score >= beta) {
                    if (legal == 1) {
                        info->failHighFirst++;
                    }
                    info->failHigh++;

                    if (!(newList->moves[moveNumber].move & MOVEFLAG_CAPTURED)) {
                        position->searchKillers[1][position->ply] = position->searchKillers[0][position->ply];
                        position->searchKillers[0][position->ply] = newList->moves[moveNumber].move;
                    }

                    storeHashEntry(position, bestMove, beta, HASH_FLAG_BETA, depth);
                    return beta;
                }
                alpha = score;
                if (!(newList->moves[moveNumber].move & MOVEFLAG_CAPTURED)) {
                    position->searchHistory[position->pieces[FROM(bestMove)]][TOSQ(bestMove)] += depth;
                }
            }
        }
    }

    if(legal == 0) {
        if(inCheck) {
            return -INFINITE + position -> ply;
        }else {
            return 0;
        }
    }

    if(alpha != oldAlpha) {
        storeHashEntry(position, bestMove, bestScore, HASH_FLAG_EXACT, depth);
    }else {
        storeHashEntry(position, bestMove, alpha, HASH_FLAG_ALPHA, depth);
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
        
        if(info -> stopped == TRUE) {
            break;
        }
        
        pvMoves = getPvLine(currentDepth, position);
        bestMove = position -> pvArray[0];

        printf("info score cp %d depth %d nodes %ld time %d ", bestScore, currentDepth, info -> nodes, getTimeInMiliseconds() - info -> startTime);

        pvMoves = getPvLine(currentDepth, position);
        printf("pv");
        for(pvNumber = 0; pvNumber < pvMoves; pvNumber++) {
            printf(" %s", printMove(position -> pvArray[pvNumber]));
        }
        printf("\n");

        //printf("Ordering: %.2f\n", (info -> failHighFirst / info -> failHigh));
    }

    printf("bestmove %s\n", printMove(bestMove));
}