CC=gcc
CFLAGS=-c -g -Wall
LIBS=-lm
SOURCE:=$(wildcard src/*.c)
# SOURCE:=$(filter-out src/fft.c, $(SOURCE))
OBJ=$(SOURCE:.c=.o)
EXE=main

.PHONY: clean clean_run

run: clean all
	./$(EXE)

all: $(SOURCE) $(EXE)

$(EXE): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJ) $(EXE)
