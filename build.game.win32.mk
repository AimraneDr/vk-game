CC = clang
CFLAGS = -Wall -g

EXE_NAME = game
BUILD_DIR = bin
OBJ_DIR = obj

DEFINES = -D_DEBUG
INCLUDE_PATH = -Iinclude -IC:/Users/aimra/Dev/c-projects/c-toolbox/include -IC:\VulkanSDK\1.3.290.0\Include
LINKER_FLAGS = -lvulkan-1 -ltoolbox -LC:/Users/aimra/Dev/libs/ -luser32 -lgdi32

SRC := $(shell dir /b /s src\*.c )
SRC := $(subst \,/,$(SRC))
SRC := $(foreach file, $(SRC), $(subst $(CURDIR), ., $(file)))

OBJ := $(patsubst ./src/%.c, ./$(OBJ_DIR)/%.o, $(SRC))


all : $(BUILD_DIR) scaffold $(EXE_NAME)

$(BUILD_DIR):
	@echo creating build directory ...
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"

scaffold :
	@echo scaffolding ... 
	$(foreach dir,$(dir $(OBJ)), \
      $(shell if not exist "$(dir)" mkdir "$(dir)") \
    )

$(EXE_NAME): $(OBJ)
	@echo building $(EXE_NAME) ...
	@$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@.exe $^ $(LINKER_FLAGS)
	@echo done.

$(OBJ_DIR)/%.o: ./src/%.c
	@echo compiling $@...
	@$(CC) $(CFLAGS) -c -o $@ $< $(INCLUDE_PATH) $(DEFINES)

clean:
	@echo cleaning ...
	@rmdir /s /q "$(BUILD_DIR)"
	@rmdir /s /q "$(OBJ_DIR)"


.PHONY: all scaffold clean

debug:
	@echo curdir : $(CURDIR)
	@echo SRC : $(SRC)
	@echo OBJ : $(OBJ)