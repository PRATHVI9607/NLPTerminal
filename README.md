# 🚀 NLPTerminal - Natural Language Processing Enhanced Shell

![Version](https://img.shields.io/badge/version-2.0-blue)
![Language](https://img.shields.io/badge/language-C-green)
![Platform](https://img.shields.io/badge/platform-Linux-orange)
![Commands](https://img.shields.io/badge/commands-86-brightgreen)

**NLPTerminal** is an advanced, educational Linux shell with natural language processing capabilities, data structure visualizers, and 86 powerful commands. Perfect for learning operating systems, data structures, and system programming.

---

## ✨ Features

### 🎯 Core Capabilities
- **86 Built-in Commands** - From basic shell operations to advanced utilities
- **Natural Language Processing** - Understand commands in plain English
- **Smart Autocomplete** - Trie-based command completion
- **Spell Checking** - BK-Tree based fuzzy matching
- **Command History** - Doubly-linked list with fast search
- **Macro System** - Record and replay command sequences
- **Undo Support** - Stack-based undo for file operations

### 📊 Educational Features
- **Data Structure Visualizers** - See linked lists, tries, trees in ASCII art
- **Expression Converters** - Infix ↔ Postfix ↔ Prefix with stack visualization
- **Algorithm Demonstrations** - Watch algorithms work step-by-step
- **System Monitoring** - Real-time OS metrics and process information

### 🛠️ Unique Commands
- **Expression Evaluation** - Mathematical expression parser and calculator
- **File Encryption** - XOR-based encryption/decryption
- **Advanced Search** - Recursive file search with pattern matching
- **Directory Tree** - Visual directory structure display
- **System Monitoring** - Advanced system statistics

---

## 📦 Installation

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt-get install gcc make libc6-dev

# Fedora/RHEL
sudo dnf install gcc make glibc-devel

# Arch Linux
sudo pacman -S gcc make
```

### Build & Install
```bash
cd backend
make clean
make
./mysh
```

### Quick Start
```bash
# Launch the shell
./mysh

# Try some commands
help                          # Show all commands
visualize trie 3              # See command tree
visualeval "5 3 + 2 *"       # Postfix calculator with visualization
macro define deploy          # Start recording macro
```

---

## 🎓 Educational Value

### Learn Operating Systems Concepts
- **Process Management** - fork(), exec(), wait(), signal handling
- **File Systems** - POSIX file operations, inodes, permissions
- **Memory Management** - Dynamic allocation, buffer management
- **I/O Systems** - File descriptors, pipes, redirections
- **Shell Mechanics** - Command parsing, execution, environment variables

### Learn Data Structures
- **Linked Lists** - Singly (macros, undo), Doubly (history)
- **Tries** - 26-ary tree for autocomplete
- **BK-Trees** - Metric space for spell checking
- **Stacks** - Character, string, and number stacks for expressions
- **Hash Tables** - File duplicate detection

### Learn Algorithms
- **Shunting Yard** - Infix to postfix conversion (Dijkstra's algorithm)
- **Expression Evaluation** - Stack-based postfix evaluation
- **Levenshtein Distance** - String similarity for spell checking
- **Tree Traversal** - DFS/BFS in trie and BK-tree
- **String Algorithms** - Pattern matching, searching

---

## 📚 Documentation

- **[USER_MANUAL.md](USER_MANUAL.md)** - Complete command reference with syntax, examples, dos/don'ts
- **[DSAreport.md](DSAreport.md)** - Data structures analysis and implementation details
- **[OSreport.md](OSreport.md)** - Operating system concepts and system calls used

---

## 🏗️ Architecture

```
NLPTerminal/
├── backend/              # C implementation
│   ├── src/             # Source files
│   │   ├── main_enhanced.c      # Main shell loop
│   │   ├── commands.c           # Basic commands
│   │   ├── custom_commands.c    # Advanced commands
│   │   ├── visualizer.c         # Data structure visualizers
│   │   ├── expression.c         # Expression converters
│   │   ├── utilities.c          # Utility commands
│   │   ├── nlp_engine.c         # NLP processing
│   │   ├── trie.c              # Autocomplete trie
│   │   ├── bktree.c            # Spell checker
│   │   ├── history.c           # Command history
│   │   ├── macros.c            # Macro system
│   │   └── undo.c              # Undo stack
│   ├── include/         # Header files
│   └── Makefile        # Build configuration
├── README.md          # This file
├── USER_MANUAL.md     # Command reference
├── DSAreport.md       # Data structures report
└── OSreport.md        # OS concepts report
```

---

## 🎮 Command Categories

- **Basic Shell**: 10 commands (ls, cd, pwd, etc.)
- **File Operations**: 12 commands (cp, mv, grep, etc.)
- **System Info**: 8 commands (date, uptime, df, etc.)
- **Process Management**: 5 commands (ps, kill, jobs, etc.)
- **History & Macros**: 5 commands (history, macro, undo)
- **NLP & Help**: 3 commands (nlp, suggest, help)
- **Advanced Custom**: 8 commands (sizeof, tree, calc, encrypt, etc.)
- **Visualizers**: 5 commands (data structure visualization)
- **Expression Processing**: 9 commands (infix/postfix conversion, evaluate)
- **Encoding**: 4 commands (base64, morse, rot13, reverse)
- **Number Conversion**: 4 commands (binary, hex, octal, baseconv)
- **File Utilities**: 5 commands (checksum, crc32, compress, diff)
- **Generators**: 3 commands (random, uuid, lorem)
- **Display**: 5 commands (colorpalette, ascii, progress, table)
- **Time**: 2 commands (timer, stopwatch)

**Total: 86 Commands**

---

## 🌟 Highlights

### 1. Visualized Expression Conversion
```bash
visualfix "a+b*c"
# Shows step-by-step infix to postfix with operator stack
Result: a b c * +
```

### 2. Data Structure Visualization
```bash
visualize macro deploy
# Shows macro as a linked list with ASCII art
```

### 3. Natural Language Processing
```bash
nlp show me files    # Executes: ls
nlp what time is it  # Executes: date
```

---

## 🧪 Testing

```bash
bash test_all_commands.sh
```
**Result**: 88/88 commands passing ✅

---

## 📖 Quick Reference

| Command | Description | Example |
|---------|-------------|---------|
| `help` | Show all commands | `help` |
| `visualize trie 3` | Show command tree | `visualize trie 3` |
| `visualeval "5 3 +"` | Postfix calculator | `visualeval "5 3 +"` |
| `sizeof *.txt` | Total file size | `sizeof *.txt` |
| `encrypt file.txt key` | Encrypt file | `encrypt data.txt pass123` |

---

## 🎯 Learning Outcomes

- ✅ Shell command parsing and execution
- ✅ POSIX system calls
- ✅ Process and memory management
- ✅ Data structure implementations
- ✅ Algorithm design and analysis
- ✅ String processing
- ✅ Error handling in C

---

**Built with ❤️ for learning**  
*Version 2.0 - January 2026*
