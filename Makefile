# mlannot build system

PREFIX  = /usr
CC      = gcc
CFLAGS  = -g -std=c99 -pedantic -Wall -Wextra -Wunused -Wwrite-strings
LDFLAGS =
EXE     = mlannot

OBJS    = mlannot.o

.PHONY: clean install

$(EXE): $(OBJS)
	$(CC) -o $(EXE) $(OBJS)

clean:
	rm -f *.o $(EXE)

install:
	install -m 755 $(EXE) $(PREFIX)/bin

.c.o:
	$(CC) $(CFLAGS) -c $<
