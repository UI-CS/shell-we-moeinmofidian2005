CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -pthread

all: shell sudoku monte_carlo

shell: shell.c
	$(CC) $(CFLAGS) shell.c -o shell

sudoku: sudoku.c
	$(CC) $(CFLAGS) sudoku.c -o sudoku $(LDFLAGS)

monte_carlo: monte_carlo.c
	$(CC) $(CFLAGS) monte_carlo.c -o monte_carlo

clean:
	rm -f shell sudoku monte_carlo