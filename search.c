#include <definitions.h>

int isRepetition(const board *position) {
    int index = 0;
    for(index = position -> historyPly - position -> isFiftyMoves; index < position -> historyPly - 1; index++) {
        ASSERT(index >= 0 && index < MAXGAMEMOVES);

        if(position -> hashKey == position -> history[index].hashKey) {
            return TRUE;
        }
    }
    return FALSE;
}

void searchPosition(board *position) {
    return;
}