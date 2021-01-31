#include "include/definitions.h"

const int hashSize = 0x100000 * 16;

int getPvLine(const int depth, board *position) {
    ASSERT(depth < MAXDEPTH);

    int move = probePvMove(position);
    int numOfMovesinArr = 0;

    while(move != NOMOVE && numOfMovesinArr < depth) {
        ASSERT(numOfMovesinArr < MAXDEPTH);

        if(moveExists(position, move)) {
            makeMove(position, move);
            position -> pvArray[numOfMovesinArr++] = move;
        }else {
            break;
        }

        move = probePvMove(position);   
    }

    while(position -> ply > 0) {
        takeMove(position);
    }
    return numOfMovesinArr;
}

void clearHashTable(hashTable *table) {
    hashEntry *p_hashEntry;
    for(p_hashEntry = table -> pTable; p_hashEntry < table -> pTable + table -> numberOfEntries; p_hashEntry++) {
        p_hashEntry -> hashKey = 0ULL;
        p_hashEntry -> move = NOMOVE;
        p_hashEntry -> depth = 0;
        p_hashEntry -> score = 0;
        p_hashEntry -> flags = 0;
    }

    table -> newWrite = 0;
}

void initHashTable(hashTable *table) {
    table -> numberOfEntries = hashSize / sizeof(hashEntry);
    table -> numberOfEntries -= 2;
    if (table->pTable != NULL) {
        free(table->pTable);
    }
    table -> pTable = (hashEntry *) malloc(table -> numberOfEntries * sizeof(hashEntry));
    clearHashTable(table);
    printf("pvTable init complete with %d entries\n", table -> numberOfEntries);
}

void storeHashEntry(board *position, const int move, int score, const int flags, const int depth) {
    int index = position -> hashKey  % position -> hashTable -> numberOfEntries; 
    ASSERT(index >= 0 && index <= position -> hashTable -> numberOfEntries - 1);
    ASSERT(depth >= 1 && depth < MAXDEPTH);
    ASSERT(flags >= HASH_FLAG_UPPER && flags <= HASH_FLAG_EXACT);
    ASSERT(score >= -INFINITE && score <= INFINITE);
    ASSERT(position -> ply >= 0 && position -> ply < MAXDEPTH);

    if(position -> hashTable -> pTable[index].hashKey == 0) {
        position -> hashTable -> newWrite++;
    }else {
        position -> hashTable -> overWrite++;
    }

    if(score > ISMATE) {
        score += position -> ply;
    }else if(score < -ISMATE) {
        score -= position -> ply;
    }

    position -> hashTable -> pTable[index].move = move;
    position -> hashTable -> pTable[index].hashKey = position -> hashKey;
    position -> hashTable -> pTable[index].flags = flags;
    position -> hashTable -> pTable[index].score = score;
    position -> hashTable -> pTable[index].depth = depth;
}

int probeHashEntry(board *position, int *move, int *score, int alpha, int beta, int depth) {
    int index = position -> hashKey  % position -> hashTable -> numberOfEntries; 
    
    ASSERT(index >= 0 && index <= position -> hashTable -> numberOfEntries - 1);
    ASSERT(depth >= 1 && depth < MAXDEPTH);
    ASSERT(alpha < beta);
    ASSERT(alpha >= -INFINITE && alpha <= INFINITE);
    ASSERT(beta >= -INFINITE && beta <= INFINITE);
    ASSERT(position -> ply >= 0 && position -> ply < MAXDEPTH);

    if(position -> hashTable -> pTable[index].hashKey == position -> hashKey) {
        *move = position -> hashTable -> pTable[index].move;
        if(position -> hashTable -> pTable[index].depth >= depth) {
            position -> hashTable -> hit++;

            ASSERT(position -> hashTable -> pTable[index].depth >= 1 && position -> hashTable -> pTable[index].depth < MAXDEPTH);
            ASSERT(position -> hashTable -> pTbale[index].flags >= HASH_FLAG_ALPHA && position -> hashTable -> pTable[index].flags <= HASH_FLAG_EXACT);
            
            *score = position->hashTable -> pTable[index].score;
            if(*score > ISMATE) {
                *score -= position -> ply;
            }else if(*score < -ISMATE) {
                *score += position -> ply;
            }

            switch(position -> hashTable -> pTable[index].flags) {
                ASSERT(*score >= -INFINITE && *score <= INFINITE);

                case HASH_FLAG_ALPHA:
                    if(*score <= alpha) {
                        *score = alpha;
                        return TRUE;
                    }
                    break;

                case HASH_FLAG_BETA:
                    if(*score >= beta) {
                        *score = beta;
                        return TRUE;
                    }
                    break;

                case HASH_FLAG_EXACT: 
                    return TRUE;
                    break;

                default:
                    ASSERT(FALSE); 
                    break;
            }
        }
    }
    return FALSE;
}

int probePvMove(const board *position) {
    int index = position -> hashKey % position -> hashTable -> numberOfEntries;
    ASSERT(index >= 0; && index <= position->hashTable->numberOfEntries - 1);
    
    if(position -> hashTable -> pTable[index].hashKey == position -> hashKey) {
        return position -> hashTable -> pTable[index].move;
    }
    return NOMOVE;
}