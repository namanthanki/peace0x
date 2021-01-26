#include <stdio.h>
#include <stdlib.h>

#ifndef DEFINITONS_H
#define DEFINITONS_H

/**************||
----Definitions||
****************/

#define NAME "peace0x 1.1"

#ifndef DEBUG
#define ASSERT(n)
#else
// #define ASSERT(n) \
// if(!(n)) { \
// printf("%s - Failed ", #n); \
// printf("On %s ", __DATE__); \
// printf("At %s ", __TIME__); \
// printf("In File %s ", __FILE__); \
// printf("At Line %d ", __LINE__); \
// exit(1);}

#define ASSERT(__x) do {                            \
        int result = (__x);                         \
        const char __name[] = #__x;                 \
        if(!result) {                               \
            fprintf(stderr,                         \
                "%s:%d %s: failed with %d\n",       \
                __FILE__, __LINE__, __name, result);\
        }                                           \
    } while(0)
#endif

#define SQUARE_NUMBER 120 // Defining maximum squares in an array
#define MAXGAMEMOVES 2048 // Defining maximum number of half moves of a game, 1024 moves
#define MAXPOSITIONMOVES 256 // Defining Maximum number of moves expected in a game
#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

/************************||
----Data Types Definition||
**************************/ 

typedef unsigned long long U64Int; // this is a 64-bit unsigned integer

/**********************||
----Enumerate Constants||
************************/

// Representing pieces as constants starting from 0 as Empty square,
// White pieces ranging from 1 to 6,
// Black pieces ranging from 7 to 12

enum { 
       EMPTY_SQR, 
       whitePwn, whiteKnight, whiteBishop, whiteRook, whiteQueen, whiteKing,
       blackPwn, blackKnight, blackBishop, blackRook, blackQueen, blackKing 
};

// Representing Rank and File as set of constant numbers 
// Representing Rank 1 to 8 as indices from 0 to 7 and No-Rank as 8
// Representing File 1 to 8 as indices from 0 to 8 and No-File as 8

enum { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, NULLFILE };
enum { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, NULLRANK };

// Representing piece colors as constant numbers

enum { WHITE, BLACK, BOTHCOLORS };

// Representing Squares of 8x8 Chess Board in Single-Dimensional Array Indices

enum {
    A1 = 21, B1, C1, D1, E1, F1, G1, H1,
    A2 = 31, B2, C2, D2, E2, F2, G2, H2,
    A3 = 41, B3, C3, D3, E3, F3, G3, H3,
    A4 = 51, B4, C4, D4, E4, F4, G4, H4,
    A5 = 61, B5, C5, D5, E5, F5, G5, H5,
    A6 = 71, B6, C6, D6, E6, F6, G6, H6,
    A7 = 81, B7, C7, D7, E7, F7, G7, H7,
    A8 = 91, B8, C8, D8, E8, F8, G8, H8,
    NULL_SQUARE, OFFBOARD
};

// Representing castling permissions through constant numbers
// 0 0 0 0 - 4bit representation 

enum { wKingSideCastle = 1, wQueenSideCastle = 2, bKingSideCastle = 4, bQueenSideCastle = 8 };

// Representing Boolean Values

enum { FALSE, TRUE };

/*************||
----Structures||
***************/

typedef struct s_Move{
    int move;
    int score;
} s_move;

typedef struct s_MoveList{
    s_move moves[MAXPOSITIONMOVES];
    int numberOfMoves;
} movelist;

typedef struct s_Undo{
    int move;
    int castlePermission;
    int isEnPassant;
    int isFiftyMoves;

    U64Int hashKey;
} undo;

typedef struct s_Board{
    int pieces[SQUARE_NUMBER];
    int numberOfPieces[13];
    int bigPieces[2];
    int majorPieces[2];
    int minorPieces[2];
    int material[2];
    int kingSquare[2];
    int pieceList[13][10]; 
    
    int boardSide;
    int isEnPassant;
    int isFiftyMoves;
    int ply;
    int historyPly;
    int castlePermission;

    U64Int pawns[3];

    U64Int hashKey;

    undo history[MAXGAMEMOVES];
} board;

/************||
----Game Move||
**************/

#define FROM(m) ((m) & 0x7f)
#define TOSQ(m) (((m) >> 7) & 0x7f)
#define CAPTURED(m) (((m) >> 14) & 0xf)
#define PROMOTED(m) (((m) >> 20) & 0xf)

#define MOVEFLAG_EnPassant 0x40000
#define MOVEFLAG_PwnStart 0x80000 
#define MOVEFLAG_CASTLED 0x1000000

#define MOVEFLAG_CAPTURED 0x7c000
#define MOVEFLAG_PROMOTION 0xf00000

/*********||
----Macros||
***********/

#define toSQUARE120(square64) (square64toSquare120[(square64)])
#define toSQUARE64(square120) (square120toSquare64[(square120)])

#define FILE_RANK_TO_SQUARE(f, r) ( (21 + (f)) + ((r) * 10) ) // takes file and rank as an input and returns number for that square
#define CLEARBIT(bitBoard, square) ((bitBoard) &= clearMask[(square)]) // Clears the bit on the given square in bitboard
#define SETBIT(bitBoard, square) ((bitBoard) |= setMask[(square)]) // Sets the bit on the given square in bitboard

#define isBQ(p) (isPieceBishopOrQueen[(p)])
#define isRQ(p) (isPieceRookOrQueen[(p)])
#define isKn(p) (isPieceKnight[(p)])
#define isKi(p) (isPieceKing[(p)])

/**********||
----Globals||
************/

extern int square120toSquare64[SQUARE_NUMBER];
extern int square64toSquare120[64];

extern U64Int setMask[64];
extern U64Int clearMask[64];
extern U64Int pieceKeys[13][120];
extern U64Int sideKey;
extern U64Int castleKey[16];

extern char pieceChar[];
extern char sideChar[];
extern char rankChar[]; 
extern char fileChar[];

extern int pieceBig[13];
extern int pieceMajor[13];
extern int pieceMinor[13];
extern int pieceValue[13];
extern int pieceColor[13];

extern int filesBoard[SQUARE_NUMBER];
extern int ranksBoard[SQUARE_NUMBER];

extern int isPiecePwn[13];
extern int isPieceKnight[13];
extern int isPieceKing[13];
extern int isPieceRookOrQueen[13];
extern int isPieceBishopOrQueen[13];

extern int pieceSlides[13];

/************||
----Functions||
**************/



/*********||
----init.c||
***********/

extern void allInit();

/**************||
----bitboards.c||
****************/

extern void printBitBoard(U64Int bitBoard);
extern int popBit(U64Int *bitBoard);
extern int countBits(U64Int bitBoard);

/*************||
----hashkeys.c||
***************/

extern U64Int generateHashKey(const board *position);

/**********||
----board.c||
************/

extern void resetBoard(board *position);
extern int parseFEN(char *fen, board *position);
extern void printBoard(const board *position);
extern void updateListsMaterial(board *position);
extern int checkBoard(const board *position);

/***********||
----attack.c||
*************/

extern int isSquareAttacked(const int square, const int boardSide, const board *position);

/*******||
----io.c||
*********/

extern char *printMove(const int move);
extern char *printSquareString(const int square);
void printMoveList(const movelist *list);

/*************||
----validate.c||
***************/

extern int squareOnBoard(const int square);
extern int boardSideValid(const int boardSide);
extern int fileRankValid(const int fileRank);
extern int pieceValidateEmpty(const int piece);
extern int pieceValidate(const int piece);

/************||
----movegen.c||
**************/

extern void generateAllMoves(const board *position, movelist *list);

/*************||
----makeMove.c||
***************/

extern int makeMove(board *position, int move);
extern void takeMove(board *position);

/**********||
----perft.c||
************/

extern void perftTest(int depth, board *position);

#endif