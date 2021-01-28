# SRC=peace0x.c 	\
# 	init.c 		\
# 	bitboards.c \
# 	hashkeys.c 	\
# 	board.c 	\
# 	data.c 		\
# 	attack.c 	\
# 	io.c 		\
# 	movegen.c 	\
# 	validate.c 	\
# 	makeMove.c 	\
# 	perft.c		\
#	search.c	
	
SRC=$(wildcard *.c)
OBJ=$(SRC:%.c=%.o)

exeext=

EXE=peace0x$(exeext)

CC=gcc
LD=gcc
CFLAGS=-fPIC  # fPIC makes code position-independent
CFLAGS_DEBUG=-g
CDEFS=-DDEBUG
INCLUDES=-I. -Iinclude
LIBS=
LDFLAGS=
RM=rm -f

default: $(EXE)

$(EXE): $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ) $(LIBS)

%.o: %.c
	$(CC) $(CDEFS) $(CFLAGS) $(CFLAGS_DEBUG) $(INCLUDES) -c $< -o $@


clean:
	$(RM) $(OBJ) $(EXE) .deps


deps: .deps

.deps:
	$(CC) -E -M $(CDEFS) $(INCLUDES) $(SRC) > $@

include .deps

.PHONY: deps clean