# NLP Terminal Makefile

CC      := gcc
CFLAGS  := -Wall -Wextra -O2 -Ibackend/include -D_GNU_SOURCE
LDFLAGS := -lm
PYTHON  := python3

SRC_DIR := backend/src
INC_DIR := backend/include
TARGET  := backend/mysh

# Exclude old main.c (using main_enhanced.c instead)
SRCS := $(filter-out $(SRC_DIR)/main.c, $(wildcard $(SRC_DIR)/*.c))
OBJS := $(SRCS:.c=.o)
HDRS := $(wildcard $(INC_DIR)/*.h)

.PHONY: all clean run shell help

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "Linking..."
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Done: $(TARGET)"

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(HDRS)
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning..."
	@rm -f $(OBJS) $(TARGET)
	@rm -rf /tmp/nlpterminal_ipc
	@find . -name "__pycache__" -type d -exec rm -rf {} + 2>/dev/null || true
	@echo "Clean."

run: $(TARGET)
	@$(PYTHON) frontend/app_multitab.py

shell: $(TARGET)
	@./$(TARGET)

help:
	@echo "NLP Terminal"
	@echo "  make        Build backend"
	@echo "  make run    Run GUI"
	@echo "  make shell  Run C shell directly"
	@echo "  make clean  Clean build files"
