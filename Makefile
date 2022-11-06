CC=g++
CFLAGS=-c -g -Wall
LIBS=-lm
SOURCE:=$(wildcard src/*.c)
# SOURCE:=$(filter-out src/fft.c, $(SOURCE))
OBJ=$(SOURCE:.c=.o)
EXE=main

.PHONY: clean clean_run

default: clean run

run: all
	./$(EXE)

all: $(SOURCE) arduino $(EXE)

arduino:
	g++ -c -g -Wall src/arduinoFFT/arduinoFFT.cpp -o src/arduinoFFT/arduinoFFT.o

$(EXE): $(OBJ)
	$(CC) $(OBJ) src/arduinoFFT/arduinoFFT.o -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f src/arduinoFFT/arduinoFFT.o
	rm -rf $(OBJ) $(EXE)
