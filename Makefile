CC := gcc
CFLAGS := -I./include/ -Wall -g
BIN_DIR := bin
OBJ_DIR := obj
SRC_COMMON := $(wildcard src/common/*.c)
SRC_SEQ := $(wildcard src/cpu_seq/*.c)
OBJ_COMMON := $(patsubst src/common/%.c, obj/common/%.o, $(SRC_COMMON))
OBJ_SEQ := $(patsubst src/cpu_seq/%.c, obj/cpu_seq/%.o, $(SRC_SEQ))

.PHONY: all clean

all: $(BIN_DIR)/cpu_seq

# Create directories
make_dirs:
	mkdir -p $(OBJ_DIR)/common
	mkdir -p $(OBJ_DIR)/cpu_seq
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/cpu_seq: $(OBJ_COMMON) $(OBJ_SEQ) | make_dirs
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/common/%.o: src/common/%.c | make_dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/cpu_seq/%.o: src/cpu_seq/%.c | make_dirs
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf bin/* obj/*
