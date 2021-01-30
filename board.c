#include "include/definitions.h"

int checkBoard(const board *position) {
    int tmpNumberOfPieces[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int tmpBigPiece[2] = { 0, 0 };
    int tmpMajorPiece[2] = { 0, 0 };
    int tmpMinorPiece[2] = { 0, 0 };
    int tmpMaterial[2] = { 0, 0 };

    int square120;
    int square64;
    int color;
    int pawnsCount;
    int tmpPiece;
    int tmpPieceNumber;

    U64Int tmpPawns[3] = { 0ULL, 0ULL, 0ULL };

    tmpPawns[WHITE] = position -> pawns[WHITE];
    tmpPawns[BLACK] = position -> pawns[BLACK];
    tmpPawns[BOTHCOLORS] = position -> pawns[BOTHCOLORS];

    for(tmpPiece = whitePwn; tmpPiece <= blackKing; tmpPiece++) {
        for(tmpPieceNumber = 0; tmpPieceNumber < position -> numberOfPieces[tmpPiece]; tmpPieceNumber++) {
            square120 = position -> pieceList[tmpPiece][tmpPieceNumber];
            ASSERT(position -> pieces[square120] == tmpPiece);
        }
    }

    for(square64 = 0; square64 < 64; square64++) {
        square120 = toSQUARE120(square64);
        tmpPiece = position -> pieces[square120];
        tmpNumberOfPieces[tmpPiece]++;
        color= pieceColor[tmpPiece];

        if(pieceBig[tmpPiece] == TRUE) {
            tmpBigPiece[color]++;
        }
        if(pieceMinor[tmpPiece] == TRUE) {
            tmpMinorPiece[color]++;
        }
        if(pieceMajor[tmpPiece] == TRUE) {
            tmpMajorPiece[color]++;
        }

        tmpMaterial[color] += pieceValue[tmpPiece];
    }

    for(tmpPiece = whitePwn; tmpPiece <= blackKing; tmpPiece++) {
        ASSERT(tmpNumberOfPieces[tmpPiece] == position -> numberOfPieces[tmpPiece]);
    }

    pawnsCount = countBits(tmpPawns[WHITE]);
    ASSERT(pawnsCount == position -> numberOfPieces[whitePwn]);
    pawnsCount = countBits(tmpPawns[BLACK]);
    ASSERT(pawnsCount == position -> numberOfPieces[blackPwn]);
    pawnsCount = countBits(tmpPawns[BOTHCOLORS]);
    ASSERT(pawnsCount == position -> numberOfPieces[blackPwn] +  position -> numberOfPieces[whitePwn]);

    while(tmpPawns[WHITE]) {
        square64 = popBit(&tmpPawns[WHITE]);
        ASSERT(position -> pieces[toSQUARE120(square64)] == whitePwn);
    }

    while(tmpPawns[BLACK]) {
        square64 = popBit(&tmpPawns[BLACK]);
        ASSERT(position -> pieces[toSQUARE120(square64)] == blackPwn);
    }

    while(tmpPawns[BOTHCOLORS]) {
        square64 = popBit(&tmpPawns[BOTHCOLORS]);
        ASSERT((position -> pieces[toSQUARE120(square64)] == blackPwn) || (position -> pieces[toSQUARE120(square64)] == whitePwn));
    }

    ASSERT(tmpMaterial[WHITE] == position -> material[WHITE] && tmpMaterial[BLACK] == position -> material[BLACK]);
    ASSERT(tmpMinorPiece[WHITE] == position -> minorPieces[WHITE] && tmpMinorPiece[BLACK] == position -> minorPieces[BLACK]);
    ASSERT(tmpMajorPiece[WHITE] == position -> majorPieces[WHITE] && tmpMajorPiece[BLACK] == position -> majorPieces[BLACK]);
    ASSERT(tmpBigPiece[WHITE] == position -> bigPieces[WHITE] && tmpBigPiece[BLACK] == position -> bigPieces[BLACK]);

    ASSERT(position -> boardSide == WHITE || position -> boardSide == BLACK);
    ASSERT(generateHashKey(position) == position -> hashKey);

    ASSERT(position -> isEnPassant == NULL_SQUARE || (ranksBoard[position -> isEnPassant] == RANK_6 && position -> boardSide == WHITE)
          || (ranksBoard[position -> isEnPassant] == RANK_3 && position -> boardSide == BLACK));

    ASSERT(position -> pieces[position -> kingSquare[WHITE]] == whiteKing);
    ASSERT(position -> pieces[position -> kingSquare[BLACK]] == blackKing);

    return TRUE;
}

void updateListsMaterial(board *position) {
    int piece;
    int square;
    int color;
    int index;

    for(index = 0; index < SQUARE_NUMBER; index++) {
        square = index;
        piece = position -> pieces[index];
        if(piece != OFFBOARD && piece != EMPTY_SQR) {
            color = pieceColor[piece];
            if(pieceBig[piece] == TRUE) {
                position -> bigPieces[color]++;
            }
            if(pieceMinor[piece] == TRUE) {
                position -> minorPieces[color]++;
            }
            if(pieceMajor[piece] == TRUE) {
                position -> majorPieces[color]++;
            }

            position -> material[color] += pieceValue[piece];

            position -> pieceList[piece][position -> numberOfPieces[piece]] = square;
            position -> numberOfPieces[piece]++;

            if(piece == whiteKing) {
                position -> kingSquare[WHITE] = square;
            }
            if(piece == blackKing) {
                position -> kingSquare[BLACK] = square;
            }

            if(piece == whitePwn) {
                SETBIT(position -> pawns[WHITE], toSQUARE64(square));
                SETBIT(position -> pawns[BOTHCOLORS], toSQUARE64(square));
            } else if(piece == blackPwn) {
                SETBIT(position -> pawns[BLACK], toSQUARE64(square));
                SETBIT(position -> pawns[BOTHCOLORS], toSQUARE64(square));
            }
        }
    }
}

int parseFEN(char *fen, board *position) {
    ASSERT(fen != NULL);
    ASSERT(position != NULL);

    int rank = RANK_8;
    int file = FILE_A;
    int piece = 0;
    int count = 0;
    int index = 0;
    int square64 = 0;
    int square120 = 0;

    resetBoard(position);

    while((rank >= RANK_1) && *fen) {
        count = 1;
        switch(*fen) {
            case 'p': piece = blackPwn;     break;
            case 'r': piece = blackRook;    break;
            case 'n': piece = blackKnight;  break;
            case 'b': piece = blackBishop;  break;
            case 'k': piece = blackKing;    break;
            case 'q': piece = blackQueen;   break;
            case 'P': piece = whitePwn;     break;
            case 'R': piece = whiteRook;    break;
            case 'N': piece = whiteKnight;  break;
            case 'B': piece = whiteBishop;  break;
            case 'K': piece = whiteKing;    break;
            case 'Q': piece = whiteQueen;   break;

            case '1': 
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                piece = EMPTY_SQR;
                count = *fen - '0';
                break;

            case '/':
            case ' ':
                rank--;
                file = FILE_A;
                fen++;
                continue;
            
            default:
                printf("FEN Error!\n");
                return -1;
        }

        for(index = 0; index < count; index++) {
            square64 = rank * 8 + file;
            square120 = toSQUARE120(square64);
            if(piece != EMPTY_SQR) {
                position -> pieces[square120] = piece;
            } 
            file++;
        }
        fen++;
    }

    ASSERT(*fen == 'w' || *fen == 'b');

    position -> boardSide = (*fen == 'w') ? WHITE : BLACK;
    fen += 2;

    for(index = 0; index < 4; index++) {
        if(*fen == ' ') {
            break;
        }

        switch(*fen) {
            case 'K': position -> castlePermission |= wKingSideCastle;      break;
            case 'Q': position -> castlePermission |= wQueenSideCastle;     break;
            case 'k': position -> castlePermission |= bKingSideCastle;      break;
            case 'q': position -> castlePermission |= bQueenSideCastle;     break;
            default: 
                break;
        }
        fen++;
    }
    fen++;

    ASSERT(position -> castlePermission >= 0 && position -> castlePermission <= 15);

    if(*fen != '-') {
        file = fen[0] - 'a';
        rank = fen[1] - '1';

        ASSERT(file >= FILE_A && file <= FILE_H);
        ASSERT(rank >= RANK_1 && rank <= RANK_8);

        position -> isEnPassant = FILE_RANK_TO_SQUARE(file, rank);
    }

    position -> hashKey = generateHashKey(position);

    updateListsMaterial(position);
    return 0;
}

void resetBoard(board *position) {
    int index = 0;
    for(index = 0; index < SQUARE_NUMBER; index++) {
        position -> pieces[index] = OFFBOARD;
    }

    for(index = 0; index < 64; index++) {
        position -> pieces[toSQUARE120(index)] = EMPTY_SQR;
    }

    for(index = 0; index < 2; index++) {
        position -> bigPieces[index] = 0;
        position -> majorPieces[index] = 0;
        position -> minorPieces[index] = 0;
        position -> material[index] = 0;
    }

    for(index = 0; index < 3; index++) {
        position -> pawns[index] = 0ULL;
    }     

    for(index = 0; index < 13; index++) {
        position -> numberOfPieces[index] = 0;
    }

    position -> kingSquare[WHITE] = position -> kingSquare[BLACK] = NULL_SQUARE;
    position -> boardSide = BOTHCOLORS;
    position -> isEnPassant = NULL_SQUARE;
    position -> isFiftyMoves = 0;
    position -> ply = 0;
    position -> historyPly = 0;
    position -> castlePermission = 0;
    position -> hashKey = 0ULL;
}

board* generateBoard() {
    board* newBoard = (board *) malloc(sizeof(board));
    newBoard -> newPvTable -> pTable = NULL;
    return newBoard;
}

void printBoard(const board *position) {
    int square;
    int file;
    int rank;
    int piece;

    printf("\nGame Board\n\n");

    for(rank = RANK_8; rank >= RANK_1; rank--) {
        printf("%d  ", rank + 1);   
        for(file = FILE_A; file <= FILE_H; file++) {
            square = FILE_RANK_TO_SQUARE(file, rank);
            piece = position -> pieces[square];
            printf("%3c", pieceChar[piece]);
        }
        printf("\n");
    }
    printf("\n   ");
    for(file = FILE_A; file <= FILE_H; file++) {
        printf("%3c", 'a' + file);
    }
    printf("\n");
    printf("Side: %c\n", sideChar[position -> boardSide]);
    printf("En Passant: %d\n", position -> isEnPassant);
    printf("Castle: %c%c%c%c\n",
            position -> castlePermission &  wKingSideCastle    ? 'K' : '-',
            position -> castlePermission &  wQueenSideCastle   ? 'Q' : '-',
            position -> castlePermission &  bKingSideCastle    ? 'k' : '-',
            position -> castlePermission &  bQueenSideCastle   ? 'q' : '-');
    printf("Hash Key: %llx\n", position -> hashKey);
}