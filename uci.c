#include "include/definitions.h"
#include <string.h>

#define INPUTBUFFER 400 * 6

void parseGo(char* line, searchInfo* info, board* position) {
	int depth = -1;
	int movesToGo = 30;
	int moveTime = -1;
	int time = -1;
	int increment = 0;
	char* ptr = NULL;
	info -> timeSet = FALSE;

	if((ptr = strstr(line, "infinite"))) {
		;
	}

	if((ptr = strstr(line, "binc")) && position -> boardSide == BLACK) {
		increment = atoi(ptr + 5);
	}

	if((ptr = strstr(line, "winc")) && position -> boardSide == WHITE) {
		increment = atoi(ptr + 5);
	}

	if ((ptr = strstr(line, "wtime")) && position -> boardSide == WHITE) {
		time = atoi(ptr + 6);
	}

	if ((ptr = strstr(line, "btime")) && position -> boardSide == BLACK) {
		time = atoi(ptr + 6);
	}

	if((ptr = strstr(line, "movestogo"))) {
		movesToGo = atoi(ptr + 10);
	}

	if ((ptr = strstr(line, "movetime"))) {
		moveTime = atoi(ptr + 9);
	}

	if ((ptr = strstr(line, "depth"))) {
		depth = atoi(ptr + 6);
	}

	if(moveTime != -1) {
		time = moveTime;
		movesToGo = 1;
	}

	info -> startTime = getTimeInMiliseconds();
	info -> depth = depth;
	
	if(time != -1) {
		info -> timeSet = TRUE;
		time /= movesToGo;
		time -= 50;
		info -> stopTime = info -> startTime + time + increment;
	}

	if (depth == -1) {
		info -> depth = MAXDEPTH;
	}

	printf("time: %d start: %d stop: %d depth: %d timeset: %d\n", time, info -> startTime, info -> stopTime, info -> depth, info -> timeSet);
	searchPosition(position, info);
}

void parsePosition(char* lineIn, board* position) {
	lineIn += 9;
	char* ptrChar = lineIn;

	if(strncmp(lineIn, "startpos", 8) == 0) {
		parseFEN(START_FEN, position);
	}else {
		ptrChar = strstr(lineIn, "fen");
		if(ptrChar == NULL) {
			parseFEN(START_FEN, position);
		}else {
			ptrChar += 4;
			parseFEN(ptrChar, position);
		}
	}

	ptrChar = strstr(lineIn, "moves");
	int move;

	if(ptrChar != NULL) {
		ptrChar += 6;
		while(*ptrChar) {
			move = parseMove(ptrChar, position);
			if(move == NOMOVE) {
				break;
			}
			makeMove(position, move);
			position -> ply = 0;

			while(*ptrChar && *ptrChar != ' ') {
				ptrChar++;
			}
			ptrChar++;
		}
	}
	printBoard(position);
}

void uciLoop() {
	setbuf(stdin, NULL);
	setbuf(stdout, NULL);

	char line[INPUTBUFFER];
	printf("id name %s\n", NAME);
	printf("id author Naman Thanki\n");
	printf("uciok\n");

	board* position = generateBoard();
	searchInfo* info;
	info = (searchInfo*) malloc(sizeof(searchInfo));
	initPvTable(position -> newPvTable);
	
	while (TRUE) {
		memset(&line[0], 0, sizeof(line));
		fflush(stdout);
		if(!fgets(line, INPUTBUFFER, stdin)) {
			continue;
		}

		if(line[0] == '\n') {
			continue;
		}

		if(!strncmp(line, "isready", 7)) {
			printf("readyok\n");
			continue;
		}else if(!strncmp(line, "position", 8)) {
			parsePosition(line, position);
		}else if(!strncmp(line, "uncinewgame", 10)) {
			parsePosition("position startpos\n", position);
		}else if(!strncmp(line, "go", 2)) {
			parseGo(line, info, position);
		}else if(!strncmp(line, "quit", 4)) { 
			info -> quit = TRUE;
			break;
		}else if(!strncmp(line, "uci", 3)) {
			printf("id name %s\n", NAME);
			printf("id author Naman Thanki\n");
			printf("uciok\n");
		}

		if(info -> quit == TRUE) {
			break;
		}

	}

	free(info);
	free(position -> newPvTable -> pTable);
}