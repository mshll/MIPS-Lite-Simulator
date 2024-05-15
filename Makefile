
CC := gcc
CFLAGS := -Wall -Iinclude
DEBUGFLAGS := -g -DDEBUG
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := .
INC_DIR := include
SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TARGET := mips_sim


all: $(BIN_DIR)/$(TARGET)

debug: CFLAGS += $(DEBUGFLAGS)
debug: $(BIN_DIR)/$(TARGET)

$(BIN_DIR)/$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c F
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)/$(TARGET)

.PHONY: all debug clean F