# NLP Terminal - User Manual

## Complete Command Reference and Usage Guide

**Version:** 1.0  
**Platform:** Linux (Ubuntu, Debian, and other distributions)

---

## Table of Contents

1. [Getting Started](#1-getting-started)
2. [File Operations](#2-file-operations)
3. [Directory Operations](#3-directory-operations)
4. [Text Processing](#4-text-processing)
5. [Search Commands](#5-search-commands)
6. [System Commands](#6-system-commands)
7. [Unique Commands](#7-unique-commands)
8. [Shell Features](#8-shell-features)
9. [Natural Language Commands](#9-natural-language-commands)
10. [Keyboard Shortcuts](#10-keyboard-shortcuts)
11. [Troubleshooting](#11-troubleshooting)

---

## 1. Getting Started

### Starting the Terminal

**Option 1: GUI Mode (Recommended)**
```bash
cd NLPTerminal
python3 frontend/app_enhanced.py
```

**Option 2: Command Line Mode**
```bash
cd NLPTerminal/backend
./mysh
```

### First Commands to Try

```bash
help              # View all available commands
pwd               # See current directory
ls                # List files
tree              # View directory structure
sysmon            # System resource monitor
```

---

## 2. File Operations

### ls - List Directory Contents

**Syntax:**
```bash
ls [options] [path]
```

**Examples:**
```bash
ls                    # List current directory
ls /home              # List specific directory
ls -la                # List with details (using system ls)
ls ..                 # List parent directory
```

**Output:**
```
file1.txt  file2.py  folder1/  folder2/
```

---

### touch - Create Empty File

**Syntax:**
```bash
touch <filename>
```

**Examples:**
```bash
touch notes.txt           # Create single file
touch file1.txt file2.txt # Create multiple files
touch .hidden             # Create hidden file
```

**Natural Language:**
```
"create file called notes.txt"
"make a new file named data.csv"
```

---

### rm - Remove File

**Syntax:**
```bash
rm <filename>
```

**Examples:**
```bash
rm old_file.txt       # Remove single file
rm temp.log           # Remove log file
```

**Warning:** This operation can be undone with the `undo` command.

**Natural Language:**
```
"delete file notes.txt"
"remove the file called temp.txt"
```

---

### cp - Copy File

**Syntax:**
```bash
cp <source> <destination>
```

**Examples:**
```bash
cp file.txt backup.txt           # Copy to same directory
cp file.txt /home/user/backup/   # Copy to different directory
cp config.ini config.ini.bak     # Create backup
```

**Natural Language:**
```
"copy file.txt to backup.txt"
"duplicate the file data.csv"
```

---

### mv - Move/Rename File

**Syntax:**
```bash
mv <source> <destination>
```

**Examples:**
```bash
mv old.txt new.txt               # Rename file
mv file.txt ../                  # Move to parent directory
mv doc.pdf /home/user/Documents/ # Move to specific path
```

**Natural Language:**
```
"rename old.txt to new.txt"
"move file.txt to the Documents folder"
```

---

### cat - Display File Contents

**Syntax:**
```bash
cat <filename>
```

**Examples:**
```bash
cat readme.txt        # Display file contents
cat config.ini        # View configuration
cat /etc/hostname     # View system file
```

**Natural Language:**
```
"show contents of readme.txt"
"display the file config.ini"
"what is in notes.txt"
```

---

## 3. Directory Operations

### pwd - Print Working Directory

**Syntax:**
```bash
pwd
```

**Example Output:**
```
/home/user/projects/NLPTerminal
```

**Natural Language:**
```
"where am i"
"current directory"
"show current path"
```

---

### cd - Change Directory

**Syntax:**
```bash
cd <path>
```

**Examples:**
```bash
cd /home              # Absolute path
cd projects           # Relative path
cd ..                 # Parent directory
cd ~                  # Home directory
cd -                  # Previous directory
cd                    # Home directory (no argument)
```

**Natural Language:**
```
"go to home"
"navigate to Documents"
"go back"
"go to parent directory"
```

---

### mkdir - Create Directory

**Syntax:**
```bash
mkdir <directory_name>
```

**Examples:**
```bash
mkdir projects            # Create single directory
mkdir src include lib     # Create multiple directories
mkdir my_folder           # Create with underscore
```

**Natural Language:**
```
"create folder called projects"
"make a new directory named backup"
```

---

### rmdir - Remove Empty Directory

**Syntax:**
```bash
rmdir <directory_name>
```

**Examples:**
```bash
rmdir old_folder          # Remove empty directory
rmdir temp                # Remove temp directory
```

**Note:** Directory must be empty. Use system `rm -r` for non-empty directories.

**Natural Language:**
```
"delete folder old_folder"
"remove directory temp"
```

---

### tree - Display Directory Tree

**Syntax:**
```bash
tree [path] [depth]
```

**Examples:**
```bash
tree                      # Current directory, default depth 4
tree /home                # Specific path
tree . 2                  # Current directory, depth 2
tree src 3                # src directory, depth 3
```

**Sample Output:**
```
.
|-- backend/
|   |-- include/
|   |   |-- commands.h
|   |   +-- utils.h
|   |-- src/
|   |   |-- main.c
|   |   +-- utils.c
|   +-- Makefile
|-- frontend/
|   |-- app.py
|   +-- app_enhanced.py
+-- README.md

3 directories, 7 files
```

**Natural Language:**
```
"show directory tree"
"display folder structure"
```

---

## 4. Text Processing

### echo - Print Text

**Syntax:**
```bash
echo <text>
```

**Examples:**
```bash
echo Hello World          # Print text
echo "Hello World"        # Print with quotes
echo $HOME                # Print environment variable
```

---

### head - Display First Lines

**Syntax:**
```bash
head <filename>
head -n <number> <filename>
```

**Examples:**
```bash
head file.txt             # First 10 lines
head -n 5 file.txt        # First 5 lines
```

---

### tail - Display Last Lines

**Syntax:**
```bash
tail <filename>
tail -n <number> <filename>
```

**Examples:**
```bash
tail log.txt              # Last 10 lines
tail -n 20 log.txt        # Last 20 lines
```

---

### wc - Word Count

**Syntax:**
```bash
wc <filename>
```

**Example:**
```bash
wc document.txt
```

**Output:**
```
  42  350  2048 document.txt
(lines words bytes filename)
```

---

## 5. Search Commands

### search - Find Text in Files

**Syntax:**
```bash
search <pattern>
```

**Examples:**
```bash
search TODO               # Find "TODO" in current directory
search error              # Find "error" in files
search "function main"    # Search for phrase
```

**Output:**
```
main.c:15: int main(int argc, char **argv) {
utils.c:42: // TODO: implement error handling
```

---

### find - Find Files by Name

**Syntax:**
```bash
find <pattern>
```

**Examples:**
```bash
find *.txt                # Find all .txt files
find config*              # Find files starting with "config"
find test                 # Find files containing "test"
```

---

### grep - Search Pattern in File

**Syntax:**
```bash
grep <pattern> <file>
```

**Examples:**
```bash
grep error log.txt        # Find "error" in log.txt
grep -i ERROR log.txt     # Case-insensitive search
grep "int main" *.c       # Search in multiple files
```

---

## 6. System Commands

### sysmon - System Resource Monitor

**Syntax:**
```bash
sysmon [options]
```

**Options:**
- No options: Full display
- `-c` or `--compact`: Compact single-line view
- `-l` or `--live`: Live updating view (Ctrl+C to exit)

**Examples:**
```bash
sysmon                    # Full system monitor
sysmon -c                 # Compact view
sysmon --live             # Live updating
```

**Sample Output:**
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
  Available:  7.4 GB
  [##########..........] 52.5%

[HDD] Disk Usage (/)
------------------------------------
  Total:    500.0 GB
  Used:     234.5 GB
  Free:     265.5 GB
  [#########...........] 46.9%

[PS] Top Processes by Memory
------------------------------------
  PID      MEM%   COMMAND
  1234     8.5%   firefox
  5678     5.2%   code
  9012     3.1%   python3
```

**Natural Language:**
```
"show system monitor"
"display cpu usage"
"memory usage"
```

---

### ps - Process List

**Syntax:**
```bash
ps
```

**Output:** Lists running processes with PID, memory %, and command.

---

### df - Disk Space

**Syntax:**
```bash
df
```

**Output:** Shows disk space usage for all mounted filesystems.

---

### uptime - System Uptime

**Syntax:**
```bash
uptime
```

**Output:**
```
System uptime: 5 days, 3 hours, 42 minutes
```

---

### env - Environment Variables

**Syntax:**
```bash
env
```

**Output:** Lists all environment variables.

---

## 7. Unique Commands

### fileinfo - Detailed File Information

**Syntax:**
```bash
fileinfo <filename>
```

**Example:**
```bash
fileinfo document.pdf
```

**Output:**
```
File Information: document.pdf
================================
Size:        1.2 MB (1258291 bytes)
Type:        PDF document
Permissions: -rw-r--r--
Owner:       user
Group:       user
Created:     2024-01-15 10:30:22
Modified:    2024-01-20 14:45:33
Accessed:    2024-01-21 09:00:00
```

---

### hexdump - Hexadecimal File Dump

**Syntax:**
```bash
hexdump <filename> [bytes]
```

**Examples:**
```bash
hexdump binary.dat            # First 256 bytes
hexdump image.png 512         # First 512 bytes
```

**Output:**
```
00000000: 89 50 4E 47 0D 0A 1A 0A  00 00 00 0D 49 48 44 52  .PNG........IHDR
00000010: 00 00 01 00 00 00 01 00  08 02 00 00 00 D3 10 3F  ...............?
```

---

### duplicate - Find Duplicate Files

**Syntax:**
```bash
duplicate [directory]
```

**Examples:**
```bash
duplicate                     # Current directory
duplicate /home/user/photos   # Specific directory
```

**Output:**
```
Scanning for duplicate files...

Duplicate Group 1 (Size: 1.5 MB):
  ./photos/IMG_001.jpg
  ./backup/IMG_001.jpg
  ./copies/photo.jpg

Duplicate Group 2 (Size: 256 KB):
  ./docs/report.pdf
  ./archive/report.pdf

Found 2 duplicate groups, 5 duplicate files
Potential space savings: 1.75 MB
```

---

### calc - Calculator

**Syntax:**
```bash
calc <expression>
```

**Examples:**
```bash
calc 2 + 3                    # Addition: 5
calc 10 * 5                   # Multiplication: 50
calc 100 / 4                  # Division: 25
calc 2 ^ 8                    # Power: 256
calc sqrt(16)                 # Square root: 4
calc sin(3.14159)             # Sine: ~0
calc log(100)                 # Log base 10: 2
```

**Supported Operations:**
- Basic: `+`, `-`, `*`, `/`, `%` (modulo)
- Power: `^` or `**`
- Functions: `sqrt`, `sin`, `cos`, `tan`, `log`, `ln`, `abs`
- Constants: `pi`, `e`

---

### quicknote - Quick Notes

**Syntax:**
```bash
quicknote <action> [content]
```

**Actions:**
- `add <note>` - Add a new note
- `list` - Show all notes
- `clear` - Remove all notes
- `delete <number>` - Delete specific note

**Examples:**
```bash
quicknote add "Remember to backup files"
quicknote add "Meeting at 3pm"
quicknote list
quicknote delete 1
quicknote clear
```

**Output (list):**
```
Quick Notes:
============
1. Remember to backup files     [2024-01-21 10:30]
2. Meeting at 3pm               [2024-01-21 11:45]
```

---

### backup - Create Backup

**Syntax:**
```bash
backup <source> [destination]
```

**Examples:**
```bash
backup important.doc                    # Creates important.doc.bak
backup config.ini /backup/config.ini    # Backup to specific location
```

---

### compare - Compare Files

**Syntax:**
```bash
compare <file1> <file2>
```

**Example:**
```bash
compare old_config.ini new_config.ini
```

**Output:**
```
Comparing files...
Line 15 differs:
  < max_connections=100
  > max_connections=200

Line 23 differs:
  < debug=false
  > debug=true

Files differ at 2 lines.
```

---

### stats - File Statistics

**Syntax:**
```bash
stats [directory]
```

**Example:**
```bash
stats /home/user/project
```

**Output:**
```
Directory Statistics: /home/user/project
=========================================
Total files:      156
Total directories: 23
Total size:       45.2 MB

By extension:
  .py     42 files    1.2 MB
  .js     38 files    890 KB
  .css    15 files    234 KB
  .html   12 files    156 KB
  Other   49 files   42.8 MB
```

---

### bookmark - Directory Bookmarks

**Syntax:**
```bash
bookmark <action> [name] [path]
```

**Actions:**
- `add <name> [path]` - Add bookmark (current dir if no path)
- `list` - Show all bookmarks
- `go <name>` - Jump to bookmarked directory
- `delete <name>` - Remove bookmark

**Examples:**
```bash
bookmark add project              # Bookmark current directory
bookmark add work /home/user/work # Bookmark specific path
bookmark list                     # Show bookmarks
bookmark go project               # Jump to bookmark
bookmark delete old_project       # Remove bookmark
```

---

### recent - Recently Modified Files

**Syntax:**
```bash
recent [count]
```

**Examples:**
```bash
recent                # Last 10 modified files
recent 20             # Last 20 modified files
```

**Output:**
```
Recently modified files:
========================
1. main.c             2 minutes ago
2. utils.c            15 minutes ago
3. config.ini         1 hour ago
4. README.md          3 hours ago
5. Makefile           1 day ago
```

---

### bulk_rename - Rename Multiple Files

**Syntax:**
```bash
bulk_rename <pattern> <replacement>
```

**Examples:**
```bash
bulk_rename .txt .md              # Change extension
bulk_rename old new               # Replace in filename
bulk_rename IMG_ Photo_           # Rename prefix
```

**Output:**
```
Renaming files...
  IMG_001.jpg -> Photo_001.jpg
  IMG_002.jpg -> Photo_002.jpg
  IMG_003.jpg -> Photo_003.jpg

Renamed 3 files.
```

---

## 8. Shell Features

### Command History

**Navigation:**
- `Up Arrow` - Previous command
- `Down Arrow` - Next command
- `history` - Show all history

**Example:**
```bash
history
```

**Output:**
```
1: ls
2: cd projects
3: cat readme.txt
4: mkdir new_folder
5: tree
```

---

### Undo Command

**Syntax:**
```bash
undo
```

**Supported Operations:**
- File creation (touch)
- File deletion (rm)
- Directory creation (mkdir)
- File copy (cp)
- File move (mv)

**Example:**
```bash
touch test.txt        # Create file
rm test.txt           # Delete file
undo                  # Restore file
```

**Output:**
```
Undoing: rm test.txt
Restored file: test.txt
```

---

### Macros

**Syntax:**
```bash
macro <action> [name]
```

**Actions:**
- `start <name>` - Start recording
- `end` - Stop recording
- `run <name>` - Execute macro
- `list` - Show all macros

**Example:**
```bash
macro start build           # Start recording
mkdir build                 # Command 1
cd build                    # Command 2
echo "Building..."          # Command 3
macro end                   # Stop recording

macro run build             # Execute all commands
```

---

### Clear Screen

**Syntax:**
```bash
clear
```

**Keyboard Shortcut:** `Ctrl+L`

---

### Exit Shell

**Syntax:**
```bash
exit
```

**Keyboard Shortcut:** `Ctrl+D`

---

## 9. Natural Language Commands

The NLP Terminal understands natural language. Here are examples:

### File Operations

| Natural Language | Translates To |
|-----------------|---------------|
| "show all files" | `ls` |
| "list files" | `ls` |
| "create file called test.txt" | `touch test.txt` |
| "delete file old.txt" | `rm old.txt` |
| "copy file.txt to backup.txt" | `cp file.txt backup.txt` |
| "rename old.txt to new.txt" | `mv old.txt new.txt` |
| "show contents of readme" | `cat readme` |

### Directory Operations

| Natural Language | Translates To |
|-----------------|---------------|
| "where am i" | `pwd` |
| "create folder called projects" | `mkdir projects` |
| "go to home" | `cd ~` |
| "go back" | `cd ..` |
| "navigate to Documents" | `cd Documents` |
| "show directory tree" | `tree` |

### System Operations

| Natural Language | Translates To |
|-----------------|---------------|
| "show system monitor" | `sysmon` |
| "cpu usage" | `sysmon` |
| "memory usage" | `sysmon` |
| "show help" | `help` |
| "clear screen" | `clear` |

---

## 10. Keyboard Shortcuts

### Navigation

| Shortcut | Action |
|----------|--------|
| `Up Arrow` | Previous command in history |
| `Down Arrow` | Next command in history |
| `Left Arrow` | Move cursor left |
| `Right Arrow` | Accept suggestion / Move cursor |
| `Tab` | Auto-complete command |
| `Home` / `Ctrl+A` | Beginning of line |
| `End` / `Ctrl+E` | End of line |

### Editing

| Shortcut | Action |
|----------|--------|
| `Ctrl+U` | Clear line before cursor |
| `Ctrl+K` | Clear line after cursor |
| `Ctrl+W` | Delete word before cursor |
| `Backspace` | Delete character before cursor |
| `Delete` | Delete character at cursor |

### Control

| Shortcut | Action |
|----------|--------|
| `Ctrl+L` | Clear screen |
| `Ctrl+C` | Cancel current command / Copy (GUI) |
| `Ctrl+D` | Exit shell |
| `Ctrl+V` | Paste (GUI) |
| `Escape` | Hide suggestions |

### Zoom (GUI Only)

| Shortcut | Action |
|----------|--------|
| `Ctrl++` / `Ctrl+=` | Zoom in |
| `Ctrl+-` | Zoom out |
| `Ctrl+0` | Reset zoom |
| `Ctrl+Scroll` | Zoom with mouse wheel |

---

## 11. Troubleshooting

### Common Issues

**Issue: "Command not found"**
```
Solution: Check spelling or use natural language.
Example: Instead of "mkdr", try "create folder" or check with "help"
```

**Issue: "Permission denied"**
```
Solution: Check file permissions with "ls -la"
Some operations may require root privileges.
```

**Issue: "Backend not connected" (GUI)**
```
Solution: 
1. Make sure backend is compiled: cd backend && make
2. Restart the frontend: python3 frontend/app_enhanced.py
```

**Issue: Suggestions not appearing**
```
Solution:
1. Start typing a command (at least 1-2 characters)
2. Press Tab for completion
3. Wait a moment for suggestions to load
```

**Issue: Undo not working**
```
Solution:
- Only file operations can be undone
- Maximum 50 operations are stored
- Some operations (like editing file contents) cannot be undone
```

### Getting Help

```bash
help                    # Show all commands
help <command>          # Help for specific command (if available)
```

### Reporting Issues

If you encounter bugs:
1. Note the command that caused the issue
2. Check the terminal for error messages
3. Try the same operation in a regular terminal
4. Report to the project maintainer

---

*NLP Terminal User Manual v1.0*
