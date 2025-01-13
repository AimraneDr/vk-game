CC = clang
CFLAGS = -Wall -g

EXE_NAME = game
BUILD_DIR = bin
OBJ_DIR = obj

DEFINES = -D_DEBUG
INCLUDE_PATH = -Iinclude -I~/Dev/c-projects/c-toolbox/include
LINKER_FLAGS = -lX11 -lvulkan -ltoolbox -L$(HOME)/Dev/libs/

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
	@$(CC) $(CFLAGS) -c -o $@ $< $(INCLUDE_PATH) $(LINKER_FLAGS) $(DEFINES)

clean:
	@echo cleaning ...
	rm -rf $(BUILD_DIR)/*
	rm -rf $(OBJ_DIR)/*

.PHONY: all scaffold clean
