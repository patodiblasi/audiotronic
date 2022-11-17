################################################################################
# Configuración básica:

CC := g++
# CFLAGS := -O3
CFLAGS := -Wall -Wextra -g
LIBS := -lm -lncurses
CONSTANTS := -D LOG_USE_COLOR -D USE_ARDUINO_FFT_MODULE=1 -D SHOW_FFMPEG_OUTPUT=0

SRC_DIR := src
BUILD_DIR := build
MAIN_PATH := $(SRC_DIR)/main.c
AUDIOTRONIC_EXE := audiotronic
EXPLICIT_OBJ := arduinoFFT.o sdl_screen.o

################################################################################
# Magia:

AUDIOTRONIC_EXE_PATH := $(BUILD_DIR)/$(AUDIOTRONIC_EXE)
SOURCES := $(shell find $(SRC_DIR) -name '*.c' -o -name '*.cpp')

SOURCES_WITHOUT_MAIN := $(filter-out $(MAIN_PATH), $(SOURCES))
ALL_OBJ := $(addsuffix .o, $(basename $(notdir $(SOURCES_WITHOUT_MAIN))))
ALL_OBJ_PATH := $(ALL_OBJ:%=$(BUILD_DIR)/%)
IMPLICIT_OBJ := $(filter-out $(EXPLICIT_OBJ), $(ALL_OBJ))
IMPLICIT_TARGETS := $(addprefix $(BUILD_DIR)/, $(IMPLICIT_OBJ))

################################################################################
# Dependencias autocalculadas para SDL:

SDLCFG := sdl2-config
CPLSDL := $(shell $(SDLCFG) --cflags)
LIBSDL := $(shell $(SDLCFG) --libs) -lSDL2_image

################################################################################
# Reglas de entrada:

.PHONY: clean run setup

default: all run

clean:
	rm -rf $(BUILD_DIR)

setup:
	@mkdir -p $(BUILD_DIR)

run:
	./$(AUDIOTRONIC_EXE_PATH)

all: setup $(ALL_OBJ_PATH) $(AUDIOTRONIC_EXE_PATH)

################################################################################
# Objetos:

$(AUDIOTRONIC_EXE_PATH): $(ALL_OBJ_PATH) $(MAIN_PATH)
	$(CC) $(CFLAGS) $(LIBS) $(CPLSDL) $(LIBSDL) $(ALL_OBJ_PATH) $(CONSTANTS) $(MAIN_PATH) -g -o $@

$(BUILD_DIR)/arduinoFFT.o: $(SRC_DIR)/arduinoFFT/arduinoFFT.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/sdl_screen.o: $(SRC_DIR)/sdl_screen.c
	$(CC) $(CFLAGS) -c $(CPLSDL) $< -o $@

.SECONDEXPANSION:

$(IMPLICIT_TARGETS): %.o: $$(addprefix $(SRC_DIR)/, $$(notdir %.c))
	$(CC) $(CFLAGS) $(CONSTANTS) -c $< -o $@

