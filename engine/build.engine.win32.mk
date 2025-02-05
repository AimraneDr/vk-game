CC = clang
CFLAGS = -Wall -g

LIB_NAME = engine
BUILD_DIR = bin
OBJ_DIR = obj
LIBS_DIR = C:\Users\aimra\Dev\libs

DEFINES = -D_DEBUG
INCLUDE_PATH = -Iinclude -IC:/Users/aimra/Dev/c-toolbox/include -IC:\VulkanSDK\1.3.290.0\Include -I$(LIBS_DIR)
LINKER_FLAGS = -L$(LIBS_DIR)  -lvulkan-1 -ltoolbox -luser32 -lgdi32

SRC := $(shell dir /b /s src\*.c)
SRC := $(subst \,/,$(SRC))
SRC := $(foreach file, $(SRC), $(subst $(CURDIR), ., $(file)))

OBJ := $(patsubst ./src/%.c, ./$(OBJ_DIR)/%.o, $(SRC))

all: $(BUILD_DIR) scaffold $(LIB_NAME) install

clean-build: clean all

$(BUILD_DIR):
	@echo creating build directory ...
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"

scaffold:
	@echo scaffolding ...
	$(foreach dir, $(dir $(OBJ)), \
		$(shell if not exist "$(dir)" mkdir "$(dir)") \
	)
$(LIB_NAME): $(OBJ)
	@echo building shared library $@ ...
	@$(CC) $(CFLAGS) -shared -o $(BUILD_DIR)/$@.dll $^ $(LINKER_FLAGS)
	@echo done.

install:
	@echo installing library to $(LIBS_DIR) ...
	@copy "$(BUILD_DIR)\$(LIB_NAME).dll" "$(LIBS_DIR)"  
	@copy "$(BUILD_DIR)\$(LIB_NAME).lib" "$(LIBS_DIR)"
	@copy "$(BUILD_DIR)\$(LIB_NAME).pdb" "$(LIBS_DIR)"
	@copy "$(BUILD_DIR)\$(LIB_NAME).exp" "$(LIBS_DIR)"
	@echo installation complete.

$(OBJ_DIR)/%.o: ./src/%.c
	@echo compiling $@...
	@$(CC) $(CFLAGS) -c -o $@ $< $(INCLUDE_PATH) $(DEFINES) 

clean:
	@echo cleaning ...
	@rmdir /s /q "$(BUILD_DIR)"
	@rmdir /s /q "$(OBJ_DIR)"

.PHONY: all scaffold clean install