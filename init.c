#include "include/definitions.h"

#define RAND_64 ((U64Int) rand() | \
                (U64Int) rand() << 15 | \
                (U64Int) rand() << 30 | \
                (U64Int) rand() << 45 | \
                ((U64Int) rand() & 0xf) << 60 )

int square120toSquare64[SQUARE_NUMBER];
int square64toSquare120[64];

int filesBoard[SQUARE_NUMBER];
int ranksBoard[SQUARE_NUMBER];

U64Int setMask[64];
U64Int clearMask[64];

U64Int pieceKeys[13][120];
U64Int sideKey;
U64Int castleKey[16];

U64Int fileBitBoardMask[8];
U64Int rankBitBoardMask[8];

U64Int blackPassedMask[64];
U64Int whitePassedMask[64];
U64Int isolatedPwnMask[64];

void initEvaluationMask() {
    int square;
    int tmpSquare;
    int file;
    int rank;

    for(square = 0; square < 8; square++) {
        fileBitBoardMask[square] = 0ULL;
        rankBitBoardMask[square] = 0ULL;
    }

    for (rank = RANK_8; rank >= RANK_1; rank--) {
        for (file = FILE_A; file <= FILE_H; file++) {
            square = rank * 8 + file;
            fileBitBoardMask[file] |= (1ULL << square);
            rankBitBoardMask[rank] |= (1ULL << square);
        }
    }

    for(square = 0; square < 64; square++) {
        blackPassedMask[square] = 0ULL;
        whitePassedMask[square] = 0ULL;
        isolatedPwnMask[square] = 0ULL;
    }

    for(square = 0; square < 64; square++) {
        tmpSquare = square + 8;
        while(tmpSquare < 64) {
            whitePassedMask[square] |= (1ULL << tmpSquare);
            tmpSquare += 8;
        }

        tmpSquare = square - 8;
        while(tmpSquare >= 0) {
            blackPassedMask[square] |= (1ULL << tmpSquare);
            tmpSquare -= 8;
        }

        if(filesBoard[toSQUARE120(square)] > FILE_A) {
            isolatedPwnMask[square] |= fileBitBoardMask[filesBoard[toSQUARE120(square)] - 1];

            tmpSquare = square + 7;
            while(tmpSquare < 64) {
                whitePassedMask[square] |= (1ULL << tmpSquare);
                tmpSquare += 8;
            }

            tmpSquare = square - 9;
            while(tmpSquare >= 0) {
                blackPassedMask[square] |= (1ULL << tmpSquare);
                tmpSquare -= 8;
            }
        }

        if(filesBoard[toSQUARE120(square)] < FILE_H) {
            isolatedPwnMask[square] |= fileBitBoardMask[filesBoard[toSQUARE120(square)] + 1];

            tmpSquare = square + 9;
            while(tmpSquare < 64) {
                whitePassedMask[square] |= (1ULL << tmpSquare);
                tmpSquare += 8;
            }

            tmpSquare = square - 7;
            while(tmpSquare >= 0) {
                blackPassedMask[square] |= (1ULL << tmpSquare);
                tmpSquare -= 8;
            }
        }
    }
}

void initFilesRanksBoard() {
    int file = FILE_A;
    int rank = RANK_1;
    int square = A1;
    int square64 = 0;
    int index = 0;

    for(index = 0; index < SQUARE_NUMBER; index++) {
        filesBoard[index] = OFFBOARD;
        ranksBoard[index] = OFFBOARD;
    }

    for(rank = RANK_1; rank <= RANK_8; rank++) {
        for(file = FILE_A; file <= FILE_H; file++) {
            square = FILE_RANK_TO_SQUARE(file, rank);
            filesBoard[square] = file;
            ranksBoard[square] = rank;
        }
    }
}

void initHashKeys() {
    int index = 0;
    int index2 = 0;
    for(index = 0; index < 13; index++) {
        for(index2 = 0; index2 < 120; index2++) {
            pieceKeys[index][index2] = RAND_64;
        }
    }
    sideKey = RAND_64;
    for(index = 0; index < 16; index++) {
        castleKey[index] = RAND_64;
    }
}

void initializeBitMasks() {
    int index = 0;
    for(index = 0; index < 64; index ++) {
        setMask[index] = 0LL;
        clearMask[index] = 0LL;
    }

    for(index = 0; index < 64; index++) {
        setMask[index] |= (1ULL << index);
        clearMask[index] = ~setMask[index];
    }
}

void initializeSquare120to64() {
    int index = 0;
    int file = FILE_A;
    int rank = RANK_1;
    int square = A1;
    int square64 = 0;

    for(index = 0; index < SQUARE_NUMBER; index++) {
        square120toSquare64[index] = 65;
    }

    for(index = 0; index < 64; index++) {
        square64toSquare120[index] = 120;
    }

    for(rank = RANK_1; rank <= RANK_8; rank++) {
        for(file = FILE_A; file <= FILE_H; file++) {
            square = FILE_RANK_TO_SQUARE(file,rank);
            square64toSquare120[square64] = square;
            square120toSquare64[square] = square64;
            square64++;
        }
    }
}

void allInit() {
    initializeSquare120to64();
    initializeBitMasks();
    initHashKeys();
    initFilesRanksBoard();
    initEvaluationMask();
    init_MVV_LVA();
}
