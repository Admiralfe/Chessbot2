CC = gcc
CFLAGS = -g -o2 -Wall
LDFLAGS =

src = $(wildcard src/*.c)
obj = $(src:.c=.o)

chessBot:	$(obj)
	$(CC) $(CFLAGS) -o $@ $^

attacks.o: attacks.h types.h

bitboard.o: bitboard.h types.h

position.o: position.h types.h bitboard.h

tests.o: tests.h position.h types.h bitboard.h

main.o: position.h bitboard.h types.h tests.h

.PHONY: clean
clean:
	rm -f $(obj) chessBot
