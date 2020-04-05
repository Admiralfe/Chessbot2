CC = gcc
CFLAGS = -g -o2 -Wall -std=c11
LDFLAGS =

src = $(wildcard src/*.c)
obj = $(src:.c=.o)

chessBot:	$(obj)
	$(CC) $(CFLAGS) -o $@ $^

attacks.o: attacks.h position.h types.h

bitboard.o: bitboard.h types.h

position.o: position.h types.h bitboard.h stack.h

stack.o: stack.h

tests.o: attacks.h movegen.h tests.h position.h types.h bitboard.h stack.h

main.o: position.h bitboard.h types.h tests.h

movegen.o: movegen.h attacks.h bitboard.h position.h types.h

tables.o: tables.h types.h bitboard.h attacks.h

.PHONY: clean
clean:
	rm -f $(obj) chessBot
