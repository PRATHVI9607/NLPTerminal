# NLP Terminal

A custom C-based shell with Natural Language Processing support and a Python/Tkinter frontend. Features intellisense-like suggestions, command history, natural language to command translation, and unique commands not found in standard UNIX.

**Platform:** Linux only (uses /proc filesystem and POSIX system calls)

---

## Table of Contents
- [Features](#features)
- [Prerequisites](#prerequisites)
- [Installation & Running](#installation--running)
- [Command Reference](#command-reference)
  - [File Operations](#file-operations)
  - [Text Processing](#text-processing)
  - [Unique Commands](#unique-commands)
  - [System Information](#system-information)
  - [Shell Features](#shell-features)
- [Natural Language Examples](#natural-language-examples)
- [Keyboard Shortcuts](#keyboard-shortcuts)

---

## Features

- **NLP Translation**: Type natural language like "show all files" and it translates to `ls`
- **Intellisense Suggestions**: Real-time command suggestions as you type
- **Command History**: Arrow keys to navigate history, Tab for completion
- **Unique Commands**: Commands not found in standard UNIX (`fileinfo`, `hexdump`, `duplicate`, etc.)
- **System Monitor**: Live system resource monitoring
- **Zoom Controls**: Ctrl+Plus/Minus to adjust font size in the UI

---

## Prerequisites

- **Linux** (Ubuntu, Debian, or any distro with /proc filesystem)
- **GCC** compiler
- **Make** build tool
- **Python 3.8+** with `tkinter` (for GUI)

Install dependencies on Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential python3 python3-tk
```

---

## Installation & Running

### Option 1: Using Makefile
```bash
# Navigate to backend directory
cd NLPTerminal/backend

# Build the project
make

# Run the shell (terminal mode)
./mysh

# Or run with Python GUI
cd ../frontend
python3 app_enhanced.py
```

### Option 2: Using Build Script
```bash
# Make script executable
chmod +x build.sh

# Build and get instructions
./build.sh
```

### Option 3: Manual Compilation
```bash
cd NLPTerminal/backend
gcc -Wall -Wextra -Iinclude -D_GNU_SOURCE -o mysh \
    src/main_enhanced.c src/utils.c src/history.c src/trie.c \
    src/bktree.c src/undo.c src/macros.c src/nlp_engine.c \
    src/suggestion_engine.c src/custom_commands.c src/sysmon_advanced.c -lm
```

---

## Command Reference

### File Operations

| Command | Description | Example |
|---------|-------------|---------|
| `ls [path]` | List directory contents | `ls` or `ls /home` |
| `pwd` | Print current directory | `pwd` |
| `cd <path>` | Change directory | `cd /home` or `cd ..` or `cd ~` |
| `mkdir <name>` | Create directory | `mkdir projects` |
| `rmdir <name>` | Remove empty directory | `rmdir old_folder` |
| `touch <file>` | Create empty file | `touch notes.txt` |
| `rm <file>` | Remove file | `rm temp.txt` |
| `cat <file>` | Display file contents | `cat readme.txt` |
| `cp <src> <dest>` | Copy file | `cp file.txt backup.txt` |
| `mv <src> <dest>` | Move/rename file | `mv old.txt new.txt` |
| `echo <text>` | Print text | `echo Hello World` |
| `tree [path]` | Directory tree view | `tree` or `tree /home` |

**Examples:**
```bash
# Create a new project structure
mkdir myproject
cd myproject
touch main.c
mkdir src include
ls

# Copy and backup files
cp main.c main_backup.c
cat main.c
```

---

### Text Processing

| Command | Description | Example |
|---------|-------------|---------|
| `head <file> [n]` | Show first n lines (default 10) | `head log.txt 5` |
| `tail <file> [n]` | Show last n lines (default 10) | `tail log.txt 20` |
| `wc <file>` | Count lines, words, chars | `wc document.txt` |
| `grep <pattern> <file>` | Search pattern in file | `grep error log.txt` |
| `sort <file>` | Sort file lines | `sort names.txt` |
| `uniq <file>` | Remove adjacent duplicates | `uniq sorted.txt` |
| `rev <file>` | Reverse each line | `rev text.txt` |

**Examples:**
```bash
# View beginning and end of a log file
head system.log 20
tail system.log 50

# Find all errors in a log
grep error application.log
grep WARNING server.log

# Count lines in a file
wc report.txt
# Output: Lines: 150  Words: 2340  Chars: 15678

# Sort and remove duplicates
sort names.txt
uniq sorted_names.txt
```

---

### Unique Commands

These commands are **not found in standard UNIX** systems:

| Command | Description | Example |
|---------|-------------|---------|
| `fileinfo <file>` | Detailed file info (size, hash, permissions, inode) | `fileinfo document.pdf` |
| `hexdump <file> [bytes]` | Display file in hexadecimal | `hexdump binary.dat 128` |
| `duplicate [path]` | Find duplicate files by content hash | `duplicate .` or `duplicate /home` |
| `encrypt <file> <key>` | Encrypt file with XOR cipher | `encrypt secret.txt mykey123` |
| `decrypt <file> <key>` | Decrypt file (same as encrypt for XOR) | `decrypt secret.txt.enc mykey123` |
| `sizeof <pattern>` | Total size of matching files | `sizeof *.txt` or `sizeof *` |
| `age <days> [older\|newer]` | Find files by age | `age 7 older` or `age 1 newer` |
| `freq <file> [top_n]` | Word frequency analysis | `freq book.txt 20` |
| `lines <file>` | Detailed line/word/char statistics | `lines code.c` |
| `quicknote [cmd]` | Quick note taking | `quicknote add "Remember to commit"` |
| `calc <expression>` | Simple calculator | `calc 2 + 3` or `calc 100 / 4` |

**Examples:**

```bash
# Get detailed info about a file
fileinfo important_document.pdf
# Output:
# === FILE INFO: important_document.pdf ===
# Size: 1048576 bytes (1.0 MB)
# Type: File
# Mode: 644
# Modified: Fri Dec 27 10:30:00 2025
# Inode: 123456
# Hash: a1b2c3d4e5f6

# View binary file in hex
hexdump program.bin 256
# Output:
# 00000000  7f 45 4c 46 02 01 01 00  00 00 00 00 00 00 00 00  |.ELF............|
# 00000010  03 00 3e 00 01 00 00 00  50 10 00 00 00 00 00 00  |..>.....P.......|

# Find duplicate files in current directory
duplicate .
# Output:
# Checking 45 files for duplicates...
# DUPLICATE: photo1.jpg <-> backup/photo1.jpg
# DUPLICATE: doc.pdf <-> old/doc.pdf

# Encrypt a file
encrypt passwords.txt secretkey
# Output: Encrypted: passwords.txt -> passwords.txt.enc

# Decrypt (XOR is symmetric)
decrypt passwords.txt.enc secretkey

# Calculate total size of all .c files
sizeof *.c
# Output: 15 files, total: 45.2 KB

# Find files modified in last 24 hours
age 1 newer
# Output:
# main.c
# test.txt
# notes.md

# Find files older than 30 days
age 30 older

# Word frequency analysis
freq novel.txt 10
# Output:
# Top 10 words:
#  152: the
#   98: and
#   76: to
#   65: a
#   54: of

# Quick notes
quicknote add "Buy groceries"
quicknote add "Call mom"
quicknote list
# Output:
# 1. Buy groceries
# 2. Call mom
quicknote clear
# Output: Notes cleared.

# Calculator
calc 15 + 27
# Output: = 42

calc 100 / 3
# Output: = 33.3333

calc 2 * 50
# Output: = 100

calc 45 - 12
# Output: = 33
```

---

### System Information

| Command | Description | Example |
|---------|-------------|---------|
| `sysmon` | Full system resource monitor | `sysmon` |
| `sysmon -c` | Compact system info | `sysmon -c` |
| `sysmon -l [sec]` | Live updating monitor | `sysmon -l 10` |
| `ps` | List running processes | `ps` |
| `kill <pid>` | Kill process by PID | `kill 1234` |
| `df` | Disk free space | `df` |
| `uptime` | System uptime | `uptime` |
| `date` | Current date/time | `date` |
| `whoami` | Current username | `whoami` |
| `hostname` | System hostname | `hostname` |

**Examples:**

```bash
# Full system monitor (CPU, Memory, Disk, Uptime)
sysmon
# Output:
# ╔════════════════════════════════════════════════════════════════╗
# ║                    SYSTEM RESOURCE MONITOR                     ║
# ╚════════════════════════════════════════════════════════════════╝
# ┌─ CPU Information ──────────────────────────────────────────────┐
# │ Processors: 8 cores
# │ CPU Usage: 23.5%
# │ [████░░░░░░░░░░░░░░░░]
# └────────────────────────────────────────────────────────────────┘
# ┌─ Memory Information ───────────────────────────────────────────┐
# │ Total: 16.00 GB  Used: 8.50 GB  Free: 7.50 GB
# │ Usage: 53%  [██████████░░░░░░░░░░]
# └────────────────────────────────────────────────────────────────┘

# Compact view (one-line summary)
sysmon -c

# Live monitor for 30 seconds (updates every second)
sysmon -l 30

# Check disk usage
df
# Output:
# Filesystem           Size       Used      Avail  Use%
# /                   100.0 GB    45.2 GB   54.8 GB   45%
# /home               500.0 GB   234.1 GB  265.9 GB   47%

# List all processes
ps
# Output:
# PID      COMMAND
# 1        systemd
# 234      sshd
# 567      bash
# 890      mysh

# Kill a process
kill 5678
# Output: Killed 5678

# Check uptime
uptime
# Output: up 5 days, 12:34

# Get current date
date
# Output: Fri Dec 27 14:30:00 2025

# Get username and hostname
whoami
# Output: john

hostname
# Output: ubuntu-desktop
```

---

### Shell Features

| Command | Description | Example |
|---------|-------------|---------|
| `history` | Show command history | `history` |
| `undo` | Undo last file operation | `undo` |
| `macro define <name>` | Start recording macro | `macro define backup` |
| `macro end` | Stop recording macro | `macro end` |
| `macro run <name>` | Run recorded macro | `macro run backup` |
| `macro list` | List available macros | `macro list` |
| `teach on/off` | Enable/disable teaching mode | `teach on` |
| `help [command]` | Show help | `help` or `help ls` |
| `clear` | Clear screen | `clear` |
| `exit` | Exit shell | `exit` |

**Examples:**

```bash
# View command history
history
# Output:
# 1. ls
# 2. cd projects
# 3. mkdir test
# 4. touch file.txt

# Undo last operation (works for mkdir, touch, cp, mv, rm)
mkdir test_folder
undo
# Undoes the mkdir

touch newfile.txt
undo
# Removes the file created

# Create and run a macro
macro define daily_backup
cp important.txt backup/important.txt
cp notes.txt backup/notes.txt
echo "Backup complete!"
macro end
# Output: Macro recording ended.

# Run the macro anytime
macro run daily_backup
# Executes all recorded commands

# Teaching mode - explains each command
teach on
ls
# Output: (shows ls results)
# [Teaching Mode] ls - Lists files and directories...

teach off

# Get help
help
# Shows all available commands

help fileinfo
# Shows detailed help for fileinfo command

# Clear screen
clear
```

---

## Natural Language Examples

Type natural language and the shell translates it to commands:

| Natural Language Input | Translated Command |
|----------------------|-------------------|
| `show all files` | `ls` |
| `what files are here` | `ls` |
| `list directory` | `ls` |
| `where am i` | `pwd` |
| `current directory` | `pwd` |
| `go to home` | `cd ~` |
| `go back` | `cd ..` |
| `go up` | `cd ..` |
| `create folder test` | `mkdir test` |
| `make directory projects` | `mkdir projects` |
| `new folder data` | `mkdir data` |
| `delete file temp.txt` | `rm temp.txt` |
| `remove temp.txt` | `rm temp.txt` |
| `show contents of readme` | `cat readme` |
| `what's in config.txt` | `cat config.txt` |
| `read file notes.txt` | `cat notes.txt` |
| `copy file.txt to backup.txt` | `cp file.txt backup.txt` |
| `rename old.txt to new.txt` | `mv old.txt new.txt` |
| `move data.txt to archive` | `mv data.txt archive` |
| `find duplicates` | `duplicate` |
| `check for duplicate files` | `duplicate` |
| `system monitor` | `sysmon` |
| `check system` | `sysmon` |
| `show resources` | `sysmon` |
| `calculate 5 plus 3` | `calc 5 + 3` |
| `word count of essay.txt` | `wc essay.txt` |
| `count lines in code.c` | `lines code.c` |
| `search for error in log` | `grep error log` |
| `find error in logfile.txt` | `grep error logfile.txt` |
| `clear screen` | `clear` |
| `show history` | `history` |
| `quit` | `exit` |

**Try it:**
```bash
# Just type naturally!
> show all files
[NLP] Translated: ls
(file listing appears)

> what files are here
[NLP] Translated: ls
(file listing appears)

> create folder myproject
[NLP] Translated: mkdir myproject
Directory 'myproject' created.

> go to myproject
[NLP] Translated: cd myproject

> system monitor
[NLP] Translated: sysmon
(system info appears)

> calculate 25 times 4
[NLP] Translated: calc 25 * 4
= 100
```

---

## Keyboard Shortcuts

### In Python GUI (app_enhanced.py)

| Shortcut | Action |
|----------|--------|
| `Enter` | Execute command |
| `Up Arrow` | Previous command from history |
| `Down Arrow` | Next command from history |
| `Tab` | Accept first suggestion |
| `Right Arrow` | Accept first suggestion (at end of input) |
| `Ctrl + +` | Zoom in (increase font size) |
| `Ctrl + -` | Zoom out (decrease font size) |
| `Ctrl + 0` | Reset zoom to default |
| `Escape` | Close suggestion popup |

### Intellisense Features

As you type, a suggestion popup appears showing:
- Matching commands
- Command descriptions
- Usage examples

Press `Tab` or `Right Arrow` to accept the first suggestion.

---

## Project Structure

```
NLPTerminal/
├── backend/
│   ├── Makefile              # Build configuration
│   ├── include/              # Header files
│   │   ├── nlp_engine.h      # NLP function declarations
│   │   ├── suggestion_engine.h
│   │   ├── custom_commands.h
│   │   ├── sysmon_advanced.h
│   │   ├── history.h
│   │   ├── trie.h
│   │   ├── bktree.h
│   │   └── ...
│   └── src/                  # Source files
│       ├── main_enhanced.c   # Main shell with NLP
│       ├── nlp_engine.c      # NLP pattern matching (30+ patterns)
│       ├── suggestion_engine.c # Command suggestions (60+ commands)
│       ├── custom_commands.c # Unique commands implementation
│       ├── sysmon_advanced.c # System monitor using /proc
│       ├── history.c         # Command history
│       ├── trie.c            # Trie for autocompletion
│       ├── bktree.c          # BK-tree for fuzzy matching
│       └── ...
├── frontend/
│   ├── app.py               # Original Python UI
│   └── app_enhanced.py      # Enhanced UI with intellisense
├── build.sh                 # Build script
└── README.md               # This file
```

---

## Troubleshooting

### Build Errors
```bash
# If make fails, try cleaning first
make clean
make

# Check GCC is installed
gcc --version
```

### Python GUI Issues
```bash
# Install tkinter if missing
sudo apt install python3-tk

# Check Python version
python3 --version
```

### Permission Denied
```bash
# Make executable
chmod +x mysh
chmod +x build.sh
```

---

## License

This project is for educational purposes.
