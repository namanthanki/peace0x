#include "include/definitions.h"

const int pvSize = 0x100000 * 2;

int getPvLine(const int depth, board *position) {
    ASSERT(depth < MAXDEPTH);

    int move = probePvTable(position);
    int numOfMovesinArr = 0;

    while(move != NOMOVE && numOfMovesinArr < depth) {
        ASSERT(numOfMovesinArr < MAXDEPTH);

        if(moveExists(position, move)) {
            makeMove(position, move);
            position -> pvArray[numOfMovesinArr++] = move;
        }else {
            break;
        }

        move = probePvTable(position);   
    }

    while(position -> ply > 0) {
        takeMove(position);
    }
    return numOfMovesinArr;
}

void clearPvTable(pvTable *table) {
    pvEntry *p_pvEntry;
    for(p_pvEntry = table -> pTable; p_pvEntry < table -> pTable + table -> numberOfEntries; p_pvEntry++) {
        p_pvEntry -> hashKey = 0ULL;
        p_pvEntry -> move = NOMOVE;
    }
}

void initPvTable(pvTable *table) {
    table -> numberOfEntries = pvSize / sizeof(pvEntry);
    table -> numberOfEntries -= 2;
    if (table->pTable != NULL) {
        free(table->pTable);
    }
    table -> pTable = (pvEntry *) malloc(table -> numberOfEntries * sizeof(pvEntry));
    clearPvTable(table);
    printf("pvTable init complete with %d entries\n", table -> numberOfEntries);
}

void storePvMove(const board *position, const int move) {
    int index = position -> hashKey  % position -> newPvTable -> numberOfEntries; 
    ASSERT(index >= 0 && index <= position -> newPvTable -> numberOfEntries - 1);

    position -> newPvTable -> pTable[index].move = move;
    position -> newPvTable -> pTable[index].hashKey = position -> hashKey;
}

int probePvTable(const board *position) {
    int index = position -> hashKey  % position -> newPvTable -> numberOfEntries; 
    ASSERT(index >= 0 && index <= position -> newPvTable -> numberOfEntries - 1);

    if(position -> newPvTable -> pTable[index].hashKey == position -> hashKey) {
        return position -> newPvTable -> pTable[index].move;
    }

    return NOMOVE;
}