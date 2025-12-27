# NLP Terminal - Linux Only Makefile
# Builds the C backend shell with all features

CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -D_GNU_SOURCE

TARGET = mysh
LDFLAGS = -lm
RM = rm -f
RMDIR = rm -rf

# Source files - Original
SRC_ORIGINAL = src/utils.c src/history.c src/trie.c src/bktree.c src/undo.c src/macros.c src/commands.c

# Source files - New enhanced modules  
SRC_ENHANCED = src/nlp_engine.c src/suggestion_engine.c src/custom_commands.c src/sysmon_advanced.c

# Main file (use enhanced version)
SRC_MAIN = src/main_enhanced.c

# All sources
SRC = $(SRC_MAIN) $(SRC_ORIGINAL) $(SRC_ENHANCED)
OBJ = $(SRC:.c=.o)

# Header files
HEADERS = include/utils.h include/history.h include/trie.h include/bktree.h include/undo.h include/macros.h \
          include/nlp_engine.h include/suggestion_engine.h include/custom_commands.h include/sysmon_advanced.h

# Default target
all: $(TARGET)

# Link
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)
	@echo "Build successful: $(TARGET)"

# Compile
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Build original shell (without enhancements)
original: src/main.o $(SRC_ORIGINAL:.c=.o)
	$(CC) $(CFLAGS) -o shell_original src/main.o $(SRC_ORIGINAL:.c=.o) $(LDFLAGS)

# Clean
clean:
	$(RM) src/*.o $(TARGET)

# Rebuild
rebuild: clean all

# Install (Unix only)
install: $(TARGET)
ifneq ($(OS),Windows_NT)
	cp $(TARGET) /usr/local/bin/
	@echo "Installed to /usr/local/bin/$(TARGET)"
endif

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: clean all

# Release build
release: CFLAGS += -O2 -DNDEBUG
release: clean all

# Run
run: $(TARGET)
	./$(TARGET)

# Help
help:
	@echo "NLP Terminal Makefile"
	@echo "====================="
	@echo ""
	@echo "Targets:"
	@echo "  all      - Build enhanced shell (default)"
	@echo "  original - Build original shell without enhancements"
	@echo "  clean    - Remove object files and executable"
	@echo "  rebuild  - Clean and rebuild"
	@echo "  debug    - Build with debug symbols"
	@echo "  release  - Build optimized release"
	@echo "  run      - Build and run"
	@echo "  install  - Install to /usr/local/bin (Unix)"
	@echo "  help     - Show this message"

.PHONY: all clean rebuild install debug release run help original
