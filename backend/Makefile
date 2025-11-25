CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
SRC = src/main.c src/utils.c src/history.c src/trie.c src/bktree.c src/undo.c src/macros.c
OBJ = $(SRC:.c=.o)
TARGET = shell

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
