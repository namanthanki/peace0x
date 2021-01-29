#include "include/definitions.h"

long leafNodes;

void perft(int depth, board *position) {
    ASSERT(checkBoard(position));

    if(depth == 0) {
        leafNodes++;
        return;
    }

    movelist newList[1];
    generateAllMoves(position, newList);

    int moveNum = 0;
    for(moveNum = 0; moveNum < newList -> numberOfMoves; moveNum++) {
        if(!makeMove(position, newList -> moves[moveNum].move)) {
            continue;
        }
        perft(depth - 1, position);
        takeMove(position);
    }
    return;
}

void perftTest(int depth, board *position) {
    ASSERT(checkBoard(position));

    printBoard(position);

    printf("\nStarting Test to Depth: %d\n", depth);

    leafNodes = 0;
    int start = getTimeInMiliseconds();

    movelist newList[1];
    generateAllMoves(position, newList);
    
    int move;
    int moveNum = 0;

    for(moveNum = 0; moveNum < newList -> numberOfMoves; moveNum++) {
        move = newList -> moves[moveNum].move;
        if(!makeMove(position, move)) {
            continue;
        }
        long cumNodes = leafNodes;
        perft(depth - 1, position);
        takeMove(position);
        long oldNodes = leafNodes - cumNodes;
        printf("Move %d: %s : %ld\n", moveNum + 1, printMove(move), oldNodes);
    }
    printf("\nTest Complete: %ld Nodes Visited in %dms\n", leafNodes, getTimeInMiliseconds() - start);
    return;
}