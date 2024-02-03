CC = clang
CFLAGS = -Wall -g

EXE_NAME = game
BUILD_DIR = bin
OBJ_DIR = obj

INCLUDE_PATH = -Iinclude
LINKER_FLAGS = -lX11

SRC :=  $(wildcard $(shell find ./src -name '*.c'))
OBJ := $(patsubst ./src/%.c, $(OBJ_DIR)/%.o, $(SRC))

all : scaffold $(EXE_NAME)

scaffold :
	@echo scaffolding ... 
	@mkdir -p $(dir $(OBJ))

$(EXE_NAME): $(OBJ)
	@echo building $(EXE_NAME) ...
	@$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $^ $(LINKER_FLAGS)
	@echo done.

$(OBJ_DIR)/%.o: ./src/%.c
	@echo compiling $@...
	@$(CC) $(CFLAGS) -c -o $@ $< $(INCLUDE_PATH)

clean:
	@echo cleaning ...
	rm -rf $(BUILD_DIR)/*
	rm -rf $(OBJ_DIR)/*

.PHONY: all scaffold clean
