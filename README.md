# NLP Terminal

<p align="center">
  <strong>A Custom C-based Shell with Natural Language Processing Support</strong>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Language-C-blue" alt="C">
  <img src="https://img.shields.io/badge/Frontend-Python%2FTkinter-green" alt="Python">
  <img src="https://img.shields.io/badge/Platform-Linux-orange" alt="Linux">
  <img src="https://img.shields.io/badge/License-MIT-yellow" alt="License">
</p>

---

## Overview

NLP Terminal is an advanced command-line shell that combines traditional Unix-like commands with natural language processing capabilities. Type commands naturally like "show all files" or "create folder called projects" and watch them translate into actual shell commands.

**Key Highlights:**
- Natural Language to Command Translation
- Intellisense-style Auto-completion
- Unique Commands not found in standard Unix
- Real-time System Resource Monitor
- Command History with Undo Support
- Modern GUI with Python/Tkinter

---

## Table of Contents

- [Features](#features)
- [Screenshots](#screenshots)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Command Reference](#command-reference)
- [Natural Language Examples](#natural-language-examples)
- [Data Structures](#data-structures)
- [Project Structure](#project-structure)
- [Documentation](#documentation)
- [Contributing](#contributing)

---

## Features

### Core Features

| Feature | Description |
|---------|-------------|
| **NLP Translation** | Type natural phrases like "show all files" → `ls` |
| **Auto-completion** | Tab completion and real-time suggestions |
| **Command History** | Navigate with Up/Down arrows |
| **Undo Operations** | Reverse file operations with `undo` |
| **System Monitor** | Live CPU, Memory, Disk monitoring |
| **Macros** | Record and replay command sequences |

### Unique Commands

Commands not found in standard Unix:

| Command | Description |
|---------|-------------|
| `fileinfo` | Detailed file information (size, hash, permissions) |
| `hexdump` | View files in hexadecimal format |
| `duplicate` | Find duplicate files by content |
| `calc` | Built-in calculator |
| `quicknote` | Quick note-taking system |
| `sysmon` | System resource monitor |
| `tree` | Visual directory tree |
| `bulk_rename` | Rename multiple files at once |

---

## Screenshots

### Terminal GUI
```
+------------------------------------------------------------------------------+
|                       NLP TERMINAL - Advanced Shell                          |
+------------------------------------------------------------------------------+
|  Features:                                                                   |
|    * Natural Language Commands - Just describe what you want!                |
|    * Auto-Complete - Tab to accept suggestions                               |
|    * System Monitor - Real-time CPU, Memory, Disk usage                      |
+------------------------------------------------------------------------------+

shell> show all files
[NLP] → ls (Listing files in current directory)
file1.txt  file2.py  folder1/  folder2/

shell> tree
.
|-- backend/
|   |-- src/
|   |   |-- main.c
|   |   +-- utils.c
|   +-- Makefile
|-- frontend/
|   +-- app.py
+-- README.md

3 directories, 5 files
```

### System Monitor
```
====================================
        SYSTEM RESOURCE MONITOR
====================================

[CPU] CPU Usage
------------------------------------
  Usage: [##########..........] 48.2%
  Cores: 8

[RAM] Memory Usage
------------------------------------
  Total:     15.6 GB
  Used:       8.2 GB (52.5%)
  [##########..........] 52.5%

[HDD] Disk Usage (/)
------------------------------------
  Total:    500.0 GB
  Used:     234.5 GB (46.9%)
```

---

## Prerequisites

### System Requirements

- **Operating System:** Linux (Ubuntu, Debian, Fedora, etc.)
- **Compiler:** GCC 7.0 or higher
- **Python:** 3.8 or higher (for GUI)
- **Libraries:** Standard C library, math library

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential python3 python3-tk
```

**Fedora:**
```bash
sudo dnf install gcc make python3 python3-tkinter
```

**Arch Linux:**
```bash
sudo pacman -S gcc make python tk
```

---

## Installation

### Quick Install

```bash
# Clone the repository
git clone https://github.com/yourusername/NLPTerminal.git
cd NLPTerminal

# Build the project
chmod +x build.sh
./build.sh

# Run the terminal
python3 frontend/app_enhanced.py
```

### Manual Build

```bash
# Navigate to backend
cd NLPTerminal/backend

# Compile with make
make clean
make

# Or compile manually
gcc -Wall -Wextra -Iinclude -D_GNU_SOURCE -o mysh \
    src/main_enhanced.c src/commands.c src/utils.c src/history.c \
    src/trie.c src/bktree.c src/undo.c src/macros.c \
    src/nlp_engine.c src/suggestion_engine.c \
    src/custom_commands.c src/sysmon_advanced.c -lm
```

---

## Quick Start

### Starting the Terminal

**GUI Mode (Recommended):**
```bash
python3 frontend/app_enhanced.py
```

**Command Line Mode:**
```bash
./backend/mysh
```

### First Commands

```bash
help              # View all commands
pwd               # Current directory
ls                # List files
tree              # Directory tree
sysmon            # System monitor
```

### Try Natural Language

```bash
"show all files"              # → ls
"create folder called test"   # → mkdir test
"where am i"                  # → pwd
"go to home"                  # → cd ~
```

---

## Command Reference

### File Operations

| Command | Syntax | Description |
|---------|--------|-------------|
| `ls` | `ls [path]` | List directory contents |
| `touch` | `touch <file>` | Create empty file |
| `rm` | `rm <file>` | Remove file |
| `cp` | `cp <src> <dest>` | Copy file |
| `mv` | `mv <src> <dest>` | Move/rename file |
| `cat` | `cat <file>` | Display file contents |

### Directory Operations

| Command | Syntax | Description |
|---------|--------|-------------|
| `pwd` | `pwd` | Print working directory |
| `cd` | `cd <path>` | Change directory |
| `mkdir` | `mkdir <name>` | Create directory |
| `rmdir` | `rmdir <name>` | Remove empty directory |
| `tree` | `tree [path] [depth]` | Display directory tree |

### System Commands

| Command | Syntax | Description |
|---------|--------|-------------|
| `sysmon` | `sysmon [-c\|-l]` | System resource monitor |
| `ps` | `ps` | List processes |
| `df` | `df` | Disk space usage |
| `uptime` | `uptime` | System uptime |

### Unique Commands

| Command | Syntax | Description |
|---------|--------|-------------|
| `fileinfo` | `fileinfo <file>` | Detailed file information |
| `hexdump` | `hexdump <file> [bytes]` | Hex view of file |
| `duplicate` | `duplicate [dir]` | Find duplicate files |
| `calc` | `calc <expr>` | Calculator |
| `quicknote` | `quicknote <cmd>` | Note taking |
| `bulk_rename` | `bulk_rename <old> <new>` | Batch rename |

### Shell Features

| Command | Syntax | Description |
|---------|--------|-------------|
| `history` | `history` | Command history |
| `undo` | `undo` | Undo last operation |
| `macro` | `macro <cmd> [name]` | Record/play macros |
| `clear` | `clear` | Clear screen |
| `help` | `help` | Show help |
| `exit` | `exit` | Exit shell |

---

## Natural Language Examples

| You Say | Shell Executes |
|---------|---------------|
| "show all files" | `ls` |
| "list files" | `ls` |
| "where am i" | `pwd` |
| "current directory" | `pwd` |
| "create folder called projects" | `mkdir projects` |
| "make a new file named test.txt" | `touch test.txt` |
| "delete file old.txt" | `rm old.txt` |
| "go to home" | `cd ~` |
| "go back" | `cd ..` |
| "show directory tree" | `tree` |
| "show system monitor" | `sysmon` |
| "copy file.txt to backup.txt" | `cp file.txt backup.txt` |
| "rename old.txt to new.txt" | `mv old.txt new.txt` |
| "show contents of readme" | `cat readme` |

---

## Data Structures

The project implements several data structures for efficient operation:

| Data Structure | Purpose | Complexity |
|---------------|---------|------------|
| **Trie** | Command auto-completion | O(m) lookup |
| **BK-Tree** | Fuzzy string matching / spell correction | O(k×m) search |
| **Doubly Linked List** | Command history navigation | O(1) traversal |
| **Stack** | Undo operations | O(1) push/pop |
| **Dynamic Array** | Fast index-based history access | O(1) access |

For detailed analysis, see [DSAreport.md](DSAreport.md).

---

## Project Structure

```
NLPTerminal/
|-- backend/
|   |-- include/           # Header files
|   |   |-- commands.h
|   |   |-- trie.h
|   |   |-- bktree.h
|   |   |-- history.h
|   |   |-- undo.h
|   |   |-- macros.h
|   |   |-- nlp_engine.h
|   |   +-- ...
|   |-- src/               # C source files
|   |   |-- main_enhanced.c
|   |   |-- commands.c
|   |   |-- trie.c
|   |   |-- bktree.c
|   |   |-- nlp_engine.c
|   |   +-- ...
|   +-- Makefile
|-- frontend/
|   |-- app_enhanced.py    # Main GUI application
|   |-- app.py             # Basic GUI
|   |-- backend_comm.py    # Backend communication
|   +-- nlp_translator.py  # NLP utilities
|-- build.sh               # Build script
|-- README.md              # This file
|-- DSAreport.md           # Data structures report
+-- user_manual.md         # Complete user manual
```

---

## Documentation

| Document | Description |
|----------|-------------|
| [README.md](README.md) | Project overview and quick start |
| [user_manual.md](user_manual.md) | Complete command reference and usage guide |
| [DSAreport.md](DSAreport.md) | Data structures and algorithms analysis |

---

## Keyboard Shortcuts

### Navigation
| Shortcut | Action |
|----------|--------|
| `Up/Down` | Navigate history |
| `Tab` | Auto-complete |
| `Right Arrow` | Accept suggestion |

### Control
| Shortcut | Action |
|----------|--------|
| `Ctrl+L` | Clear screen |
| `Ctrl+C` | Cancel/Copy |
| `Ctrl+D` | Exit |

### GUI Zoom
| Shortcut | Action |
|----------|--------|
| `Ctrl++` | Zoom in |
| `Ctrl+-` | Zoom out |
| `Ctrl+0` | Reset zoom |

---

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

---

## License

This project is licensed under the MIT License - see the LICENSE file for details.

---

## Acknowledgments

- Built with C and Python
- GUI powered by Tkinter
- Inspired by modern terminal emulators and IDEs

---

<p align="center">
  <strong>NLP Terminal</strong> - Making the command line more natural
</p>
