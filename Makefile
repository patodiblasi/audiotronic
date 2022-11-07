CC=g++
CFLAGS=-c -g -Wall
LIBS=-lm
SOURCE:=$(wildcard src/*.c)
# SOURCE:=$(filter-out src/fft.c, $(SOURCE))
OBJ=$(SOURCE:.c=.o)
EXE=main

SDLCFG := sdl2-config
CPLSDL = $(shell $(SDLCFG) --cflags)
LIBSDL = $(shell $(SDLCFG) --libs)
LIBSDL += -lSDL2_image

CONSTANTS=-D SHOW_FFMPEG_OUTPUT=0 -D USE_ARDUINO_FFT_MODULE=1

.PHONY: clean run

default: clean all run

run:
	./$(EXE)

all: $(SOURCE) arduino $(EXE)

arduino:
	g++ -c -g -Wall src/arduinoFFT/arduinoFFT.cpp -o src/arduinoFFT/arduinoFFT.o

$(EXE): $(OBJ)
	$(CC) $(CONSTANTS) $(OBJ) $(LIBS) $(CPLSDL) $(LIBSDL) src/arduinoFFT/arduinoFFT.o -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(CONSTANTS) $(CPLSDL) $(LIBSDL) $< -o $@

clean:
	rm -f src/arduinoFFT/arduinoFFT.o
	rm -rf $(OBJ) $(EXE)
