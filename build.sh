#!/bin/bash
# NLP Terminal - Linux Build Script

echo "=== Building NLP Terminal Backend ==="
cd "$(dirname "$0")/backend"

# Clean previous build
echo "[1/3] Cleaning..."
rm -f *.o src/*.o mysh

# Compile all source files
echo "[2/3] Compiling..."
gcc -Wall -Wextra -Iinclude -D_GNU_SOURCE -c src/utils.c -o src/utils.o
gcc -Wall -Wextra -Iinclude -D_GNU_SOURCE -c src/history.c -o src/history.o
gcc -Wall -Wextra -Iinclude -D_GNU_SOURCE -c src/trie.c -o src/trie.o
gcc -Wall -Wextra -Iinclude -D_GNU_SOURCE -c src/bktree.c -o src/bktree.o
gcc -Wall -Wextra -Iinclude -D_GNU_SOURCE -c src/undo.c -o src/undo.o
gcc -Wall -Wextra -Iinclude -D_GNU_SOURCE -c src/macros.c -o src/macros.o
gcc -Wall -Wextra -Iinclude -D_GNU_SOURCE -c src/commands.c -o src/commands.o
gcc -Wall -Wextra -Iinclude -D_GNU_SOURCE -c src/nlp_engine.c -o src/nlp_engine.o
gcc -Wall -Wextra -Iinclude -D_GNU_SOURCE -c src/suggestion_engine.c -o src/suggestion_engine.o
gcc -Wall -Wextra -Iinclude -D_GNU_SOURCE -c src/custom_commands.c -o src/custom_commands.o
gcc -Wall -Wextra -Iinclude -D_GNU_SOURCE -c src/sysmon_advanced.c -o src/sysmon_advanced.o
gcc -Wall -Wextra -Iinclude -D_GNU_SOURCE -c src/main_enhanced.c -o src/main_enhanced.o

# Link
echo "[3/3] Linking..."
gcc -o mysh src/main_enhanced.o src/utils.o src/history.o src/trie.o src/bktree.o src/undo.o src/macros.o src/commands.o src/nlp_engine.o src/suggestion_engine.o src/custom_commands.o src/sysmon_advanced.o -lm

if [ -f mysh ]; then
    echo "=== Build successful! ==="
    echo "Run with: ./backend/mysh"
    echo ""
    echo "To use with Python frontend:"
    echo "  python3 frontend/app_enhanced.py"
else
    echo "=== Build failed! ==="
    exit 1
fi
