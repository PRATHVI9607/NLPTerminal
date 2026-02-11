# 📖 NLPTerminal User Manual

**Complete Command Reference with Syntax, Examples, and Best Practices**

---

## Table of Contents

1. [Basic Shell Commands](#1-basic-shell-commands-10)
2. [File Operations](#2-file-operations-12)
3. [System Information](#3-system-information-8)
4. [Process Management](#4-process-management-5)
5. [History & Macros](#5-history--macros-5)
6. [NLP & Help](#6-nlp--help-3)
7. [Advanced Custom Commands](#7-advanced-custom-commands-8)
8. [Data Structure Visualizers](#8-data-structure-visualizers-5)
9. [Expression Processing](#9-expression-processing-9)
10. [Encoding/Decoding](#10-encodingdecoding-4)
11. [Number Conversions](#11-number-conversions-4)
12. [File Utilities](#12-file-utilities-5)
13. [Generators](#13-generators-3)
14. [Display Utilities](#14-display-utilities-5)
15. [Time Utilities](#15-time-utilities-2)

---

## 1. Basic Shell Commands (10)

### `ls` - List Directory Contents
**Syntax**: `ls [path]`

**Description**: Lists files and directories in the specified path.

**Examples**:
```bash
ls                  # List current directory
ls /home           # List /home directory
ls /tmp            # List /tmp directory
```

**✅ Do**:
- Use for quick directory browsing
- Check contents before operations

**❌ Don't**:
- Don't expect flags like -la (not implemented)

---

### `cd` - Change Directory
**Syntax**: `cd <path>`

**Description**: Changes the current working directory.

**Examples**:
```bash
cd /home/user      # Absolute path
cd ..              # Parent directory
cd ~               # Home directory
cd /               # Root directory
```

**✅ Do**:
- Use absolute paths for clarity
- Use ~ for home directory

**❌ Don't**:
- Don't use without arguments (stays in current dir)

---

### `pwd` - Print Working Directory
**Syntax**: `pwd`

**Description**: Displays the current working directory.

**Examples**:
```bash
pwd                # Shows current path
```

**✅ Do**:
- Use to confirm location before operations

**❌ Don't**:
- Don't expect environment variable output

---

### `mkdir` - Make Directory
**Syntax**: `mkdir <directory>`

**Description**: Creates a new directory.

**Examples**:
```bash
mkdir newdir       # Create directory
mkdir /tmp/test    # Create with path
```

**✅ Do**:
- Create directories before operations
- Use absolute paths when needed

**❌ Don't**:
- Don't create without checking if exists
- No -p flag for recursive creation

---

### `rmdir` - Remove Directory
**Syntax**: `rmdir <directory>`

**Description**: Removes an empty directory.

**Examples**:
```bash
rmdir olddir       # Remove empty directory
```

**✅ Do**:
- Ensure directory is empty first

**❌ Don't**:
- Don't try to remove non-empty directories
- Use `rm` for files

---

### `touch` - Create File
**Syntax**: `touch <filename>`

**Description**: Creates an empty file or updates timestamp.

**Examples**:
```bash
touch file.txt     # Create empty file
touch test.c       # Create source file
```

**✅ Do**:
- Create placeholder files
- Update file timestamps

**❌ Don't**:
- Don't overwrite existing content

---

### `cat` - Display File Contents
**Syntax**: `cat <filename>`

**Description**: Displays the contents of a file.

**Examples**:
```bash
cat file.txt       # Show file contents
cat /etc/hosts     # Show system file
```

**✅ Do**:
- View small text files
- Check file contents

**❌ Don't**:
- Don't use on binary files
- Don't use on very large files

---

### `echo` - Print Text
**Syntax**: `echo <text>`

**Description**: Prints text to the terminal.

**Examples**:
```bash
echo Hello         # Print hello
echo "Test 123"    # Print with spaces
```

**✅ Do**:
- Display messages
- Debug output

**❌ Don't**:
- Don't expect variable expansion

---

### `exit` - Exit Shell
**Syntax**: `exit`

**Description**: Exits the NLPTerminal shell.

**Examples**:
```bash
exit               # Close shell
```

**✅ Do**:
- Use to close shell cleanly

**❌ Don't**:
- Don't use Ctrl+C (may corrupt state)

---

### `clear` - Clear Screen
**Syntax**: `clear`

**Description**: Clears the terminal screen.

**Examples**:
```bash
clear              # Clear screen
```

**✅ Do**:
- Clean up cluttered output

**❌ Don't**:
- Don't use excessively

---

## 2. File Operations (12)

### `cp` - Copy Files
**Syntax**: `cp <source> <destination>`

**Description**: Copies files to a new location.

**Examples**:
```bash
cp file.txt backup.txt        # Copy file
cp /tmp/test.c ./test.c       # Copy from path
```

**✅ Do**:
- Back up important files
- Use absolute paths

**❌ Don't**:
- Don't copy to same name in same directory
- Not recursive for directories

---

### `mv` - Move/Rename Files
**Syntax**: `mv <source> <destination>`

**Description**: Moves or renames files.

**Examples**:
```bash
mv old.txt new.txt            # Rename file
mv file.txt /tmp/             # Move file
```

**✅ Do**:
- Rename files safely
- Organize file structure

**❌ Don't**:
- Don't move system files
- Check destination exists

---

### `rm` - Remove Files
**Syntax**: `rm <filename>`

**Description**: Deletes a file (with undo support).

**Examples**:
```bash
rm temp.txt        # Delete file
rm /tmp/test       # Delete with path
```

**✅ Do**:
- Use undo if mistake made
- Verify before deletion

**❌ Don't**:
- Don't delete system files
- No -rf flags available

---

### `wc` - Word Count
**Syntax**: `wc <filename>`

**Description**: Counts lines, words, and characters in a file.

**Examples**:
```bash
wc file.txt        # Count statistics
wc program.c       # Count code lines
```

**✅ Do**:
- Check file size
- Count code metrics

**❌ Don't**:
- Don't use on binary files

---

### `head` - Show File Start
**Syntax**: `head <filename>`

**Description**: Displays the first 10 lines of a file.

**Examples**:
```bash
head log.txt       # Show first 10 lines
head data.csv      # Preview data
```

**✅ Do**:
- Preview large files
- Check file format

**❌ Don't**:
- Don't expect line count option

---

### `tail` - Show File End
**Syntax**: `tail <filename>`

**Description**: Displays the last 10 lines of a file.

**Examples**:
```bash
tail log.txt       # Show last 10 lines
tail error.log     # Check recent errors
```

**✅ Do**:
- Monitor log files
- Check recent entries

**❌ Don't**:
- Don't expect -f flag for following

---

### `grep` - Search in Files
**Syntax**: `grep <pattern> <filename>`

**Description**: Searches for a pattern in a file.

**Examples**:
```bash
grep error log.txt           # Find errors
grep "TODO" src/*.c          # Search TODO comments
```

**✅ Do**:
- Search for specific text
- Find patterns in logs

**❌ Don't**:
- Don't use complex regex (basic matching only)

---

### `find` - Find Files
**Syntax**: `find <path> -name <pattern>`

**Description**: Searches for files by name.

**Examples**:
```bash
find . -name "*.c"           # Find C files
find /tmp -name test         # Find by name
```

**✅ Do**:
- Locate files recursively
- Use wildcards in patterns

**❌ Don't**:
- Don't search entire filesystem (slow)

---

### `chmod` - Change Permissions
**Syntax**: `chmod <mode> <filename>`

**Description**: Changes file permissions.

**Examples**:
```bash
chmod 755 script.sh          # Make executable
chmod 644 file.txt           # Read/write owner
```

**✅ Do**:
- Make scripts executable
- Secure sensitive files

**❌ Don't**:
- Don't chmod system files
- Use octal notation only

---

### `chown` - Change Owner
**Syntax**: `chown <user> <filename>`

**Description**: Changes file ownership.

**Examples**:
```bash
chown user file.txt          # Change owner
```

**✅ Do**:
- Use with proper permissions

**❌ Don't**:
- Don't chown without sudo (may fail)

---

### `ln` - Create Link
**Syntax**: `ln -s <target> <link>`

**Description**: Creates a symbolic link.

**Examples**:
```bash
ln -s /usr/bin/app link      # Create symlink
```

**✅ Do**:
- Create shortcuts to files
- Link executables

**❌ Don't**:
- Don't create circular links

---

### `stat` - File Statistics
**Syntax**: `stat <filename>`

**Description**: Displays detailed file information.

**Examples**:
```bash
stat file.txt      # Show file stats
```

**✅ Do**:
- Check file metadata
- Verify permissions

**❌ Don't**:
- Don't use on non-existent files

---

## 3. System Information (8)

### `date` - Show Date/Time
**Syntax**: `date`

**Description**: Displays current date and time.

**Examples**:
```bash
date               # Show current time
```

**✅ Do**:
- Check system time

**❌ Don't**:
- Don't expect formatting options

---

### `uptime` - System Uptime
**Syntax**: `uptime`

**Description**: Shows how long system has been running.

**Examples**:
```bash
uptime             # Show uptime
```

**✅ Do**:
- Monitor system stability

**❌ Don't**:
- Values may vary by system

---

### `whoami` - Current User
**Syntax**: `whoami`

**Description**: Displays the current username.

**Examples**:
```bash
whoami             # Show username
```

**✅ Do**:
- Verify current user
- Check permissions context

**❌ Don't**:
- Not for full user info

---

### `uname` - System Information
**Syntax**: `uname`

**Description**: Shows system information.

**Examples**:
```bash
uname              # Show OS name
```

**✅ Do**:
- Check OS version

**❌ Don't**:
- Limited to basic info

---

### `hostname` - Show Hostname
**Syntax**: `hostname`

**Description**: Displays the system hostname.

**Examples**:
```bash
hostname           # Show hostname
```

**✅ Do**:
- Identify system

**❌ Don't**:
- Don't use to change hostname

---

### `df` - Disk Usage
**Syntax**: `df`

**Description**: Shows disk space usage.

**Examples**:
```bash
df                 # Show disk usage
```

**✅ Do**:
- Check available space

**❌ Don't**:
- Output format is basic

---

### `du` - Directory Size
**Syntax**: `du <directory>`

**Description**: Shows directory size.

**Examples**:
```bash
du /home           # Home size
du .               # Current dir size
```

**✅ Do**:
- Find large directories

**❌ Don't**:
- May be slow on large dirs

---

### `free` - Memory Usage
**Syntax**: `free`

**Description**: Displays memory usage.

**Examples**:
```bash
free               # Show memory
```

**✅ Do**:
- Monitor memory

**❌ Don't**:
- Format is basic

---

## 4. Process Management (5)

### `ps` - Process Status
**Syntax**: `ps`

**Description**: Lists running processes.

**Examples**:
```bash
ps                 # Show processes
```

**✅ Do**:
- Check running processes

**❌ Don't**:
- Limited options available

---

### `top` - Process Monitor
**Syntax**: `top`

**Description**: Interactive process viewer.

**Examples**:
```bash
top                # Monitor processes
```

**✅ Do**:
- Monitor CPU/memory

**❌ Don't**:
- Press q to exit

---

### `kill` - Terminate Process
**Syntax**: `kill <PID>`

**Description**: Terminates a process by PID.

**Examples**:
```bash
kill 1234          # Kill process 1234
```

**✅ Do**:
- Use correct PID

**❌ Don't**:
- Don't kill system processes

---

### `jobs` - Background Jobs
**Syntax**: `jobs`

**Description**: Lists background jobs.

**Examples**:
```bash
jobs               # Show jobs
```

**✅ Do**:
- Check background tasks

**❌ Don't**:
- Limited job control

---

### `bg` - Background Process
**Syntax**: `bg`

**Description**: Resumes background job.

**Examples**:
```bash
bg                 # Resume job
```

**✅ Do**:
- Manage suspended jobs

**❌ Don't**:
- Limited functionality

---

## 5. History & Macros (5)

### `history` - Command History
**Syntax**: `history`

**Description**: Shows command history.

**Examples**:
```bash
history            # Show all history
```

**✅ Do**:
- Review past commands

**❌ Don't**:
- Limited to session

---

### `history_search` - Search History
**Syntax**: `history_search <pattern>`

**Description**: Searches command history.

**Examples**:
```bash
history_search ls           # Find ls commands
history_search git          # Find git commands
```

**✅ Do**:
- Find past commands quickly

**❌ Don't**:
- Case-sensitive search

---

### `macro define` - Record Macro
**Syntax**: `macro define <name>`

**Description**: Starts recording a macro.

**Examples**:
```bash
macro define build          # Start recording
ls
make
macro end                   # Stop recording
```

**✅ Do**:
- Automate repetitive tasks
- Use descriptive names

**❌ Don't**:
- Don't nest macros

---

### `macro run` - Execute Macro
**Syntax**: `macro run <name>`

**Description**: Executes a saved macro.

**Examples**:
```bash
macro run build             # Run build macro
```

**✅ Do**:
- Execute saved sequences

**❌ Don't**:
- Macro must exist

---

### `macro list` - List Macros
**Syntax**: `macro list`

**Description**: Lists all saved macros.

**Examples**:
```bash
macro list                  # Show macros
```

**✅ Do**:
- Check available macros

**❌ Don't**:
- N/A

---

## 6. NLP & Help (3)

### `nlp` - Natural Language Processing
**Syntax**: `nlp <natural_language_query>`

**Description**: Processes natural language commands.

**Examples**:
```bash
nlp show me files           # Executes: ls
nlp what time is it         # Executes: date
nlp show current directory  # Executes: pwd
```

**✅ Do**:
- Use simple English phrases
- Be descriptive

**❌ Don't**:
- Don't use complex sentences
- Not all phrases recognized

---

### `suggest` - Command Suggestions
**Syntax**: `suggest <prefix>`

**Description**: Suggests commands matching prefix.

**Examples**:
```bash
suggest l                   # Suggests ls, ln, lorem, etc.
suggest vi                  # Suggests visualize commands
```

**✅ Do**:
- Discover commands

**❌ Don't**:
- Limited to prefix matching

---

### `help` - Show Help
**Syntax**: `help`

**Description**: Displays all available commands.

**Examples**:
```bash
help                        # Show all commands
```

**✅ Do**:
- Learn available commands

**❌ Don't**:
- No per-command help

---

## 7. Advanced Custom Commands (8)

### `sizeof` - Calculate Total Size
**Syntax**: `sizeof <pattern>`

**Description**: Calculates total size of files matching pattern.

**Examples**:
```bash
sizeof *.c                  # Total size of C files
sizeof /tmp/*               # Total size in tmp
```

**✅ Do**:
- Use wildcards
- Check disk usage

**❌ Don't**:
- Don't use on too many files

---

### `tree` - Directory Tree
**Syntax**: `tree <path> <depth>`

**Description**: Displays directory structure as tree.

**Examples**:
```bash
tree . 2                    # Tree depth 2
tree /home 3                # Home tree depth 3
```

**✅ Do**:
- Visualize directory structure
- Limit depth for clarity

**❌ Don't**:
- Don't use large depths (slow)

---

### `calc` - Calculator
**Syntax**: `calc <num1> <op> <num2>`

**Description**: Performs basic arithmetic.

**Examples**:
```bash
calc 5 + 3                  # = 8
calc 10 * 2                 # = 20
calc 15 / 3                 # = 5
calc 10 - 2                 # = 8
```

**✅ Do**:
- Quick calculations
- Check division by zero

**❌ Don't**:
- Only binary operations
- No complex expressions

---

### `backup` - Backup File
**Syntax**: `backup <filename>`

**Description**: Creates a backup with .bak extension.

**Examples**:
```bash
backup important.txt        # Creates important.txt.bak
```

**✅ Do**:
- Back up before editing

**❌ Don't**:
- Overwrites existing .bak

---

### `encrypt` - Encrypt File
**Syntax**: `encrypt <file> <key>`

**Description**: XOR-based file encryption.

**Examples**:
```bash
encrypt secret.txt mykey123     # Creates secret.txt.enc
```

**✅ Do**:
- Use strong keys
- Remember your key!

**❌ Don't**:
- Not military-grade encryption
- Don't lose the key

---

### `decrypt` - Decrypt File
**Syntax**: `decrypt <file.enc> <key>`

**Description**: Decrypts encrypted file.

**Examples**:
```bash
decrypt secret.txt.enc mykey123  # Creates secret.txt
```

**✅ Do**:
- Use same key as encryption

**❌ Don't**:
- Wrong key = corrupted file

---

### `search` - Recursive Search
**Syntax**: `search <path> <pattern>`

**Description**: Recursively searches for pattern in files.

**Examples**:
```bash
search /home TODO           # Find TODO in /home
search . error              # Find error in current dir
```

**✅ Do**:
- Search code repositories
- Find specific text

**❌ Don't**:
- May be slow on large dirs

---

### `duplicate` - Find Duplicates
**Syntax**: `duplicate <directory>`

**Description**: Finds duplicate files by hash.

**Examples**:
```bash
duplicate .                 # Find duplicates here
duplicate /home/user/Downloads  # Find in Downloads
```

**✅ Do**:
- Clean up duplicates
- Free disk space

**❌ Don't**:
- Slow on large directories

---

## 8. Data Structure Visualizers (5)

### `visualize macro` - Show Macro Structure
**Syntax**: `visualize macro <name>`

**Description**: Displays macro as linked list.

**Examples**:
```bash
visualize macro build       # Show build macro structure
```

**✅ Do**:
- Learn linked list structure
- See macro steps

**❌ Don't**:
- Macro must exist first

---

### `visualize history` - Show History Structure
**Syntax**: `visualize history`

**Description**: Displays history as doubly-linked list.

**Examples**:
```bash
visualize history           # Show history structure
```

**✅ Do**:
- Understand history implementation

**❌ Don't**:
- Requires commands in history

---

### `visualize undo` - Show Undo Stack
**Syntax**: `visualize undo`

**Description**: Displays undo stack structure.

**Examples**:
```bash
visualize undo              # Show undo stack
```

**✅ Do**:
- Learn stack data structure

**❌ Don't**:
- Requires undo operations

---

### `visualize trie` - Show Command Trie
**Syntax**: `visualize trie <depth>`

**Description**: Displays command trie tree.

**Examples**:
```bash
visualize trie 2            # Show trie depth 2
visualize trie 3            # Show trie depth 3
```

**✅ Do**:
- Learn trie structure
- See autocomplete mechanism

**❌ Don't**:
- Large depths = cluttered output

---

### `visualize bktree` - Show BK-Tree
**Syntax**: `visualize bktree <distance>`

**Description**: Displays spell-checker BK-tree.

**Examples**:
```bash
visualize bktree 2          # Show tree with distance 2
```

**✅ Do**:
- Understand spell checking

**❌ Don't**:
- Large distance = many nodes

---

## 9. Expression Processing (9)

### `infix2postfix` - Convert to Postfix
**Syntax**: `infix2postfix "<expression>"`

**Description**: Converts infix to postfix notation.

**Examples**:
```bash
infix2postfix "a+b*c"       # → a b c * +
infix2postfix "(a+b)*(c-d)" # → a b + c d - *
```

**✅ Do**:
- Use quotes for expressions
- Include operators: + - * / ^ %

**❌ Don't**:
- Don't forget quotes

---

### `infix2prefix` - Convert to Prefix
**Syntax**: `infix2prefix "<expression>"`

**Description**: Converts infix to prefix notation.

**Examples**:
```bash
infix2prefix "a+b*c"        # → + a * b c
```

**✅ Do**:
- Use for Polish notation

**❌ Don't**:
- Don't forget quotes

---

### `postfix2infix` - Postfix to Infix
**Syntax**: `postfix2infix "<expression>"`

**Description**: Converts postfix to infix.

**Examples**:
```bash
postfix2infix "a b + c *"   # → ((a+b)*c)
```

**✅ Do**:
- Use space-separated tokens

**❌ Don't**:
- Don't omit spaces

---

### `prefix2infix` - Prefix to Infix
**Syntax**: `prefix2infix "<expression>"`

**Description**: Converts prefix to infix.

**Examples**:
```bash
prefix2infix "+ a * b c"    # → (a+(b*c))
```

**✅ Do**:
- Use space-separated tokens

**❌ Don't**:
- Don't omit spaces

---

### `evaluate` - Evaluate Postfix
**Syntax**: `evaluate "<postfix_expression>"`

**Description**: Evaluates postfix expression.

**Examples**:
```bash
evaluate "5 3 +"            # = 8
evaluate "5 3 + 2 *"        # = 16
evaluate "10 5 + 3 *"       # = 45
```

**✅ Do**:
- Use space-separated tokens
- Numbers and operators only

**❌ Don't**:
- Don't use variables
- Check for division by zero

---

### `exprtype` - Detect Expression Type
**Syntax**: `exprtype "<expression>"`

**Description**: Detects if expression is infix, postfix, or prefix.

**Examples**:
```bash
exprtype "a+b*c"            # → INFIX
exprtype "a b + c *"        # → POSTFIX
```

**✅ Do**:
- Use to identify notation

**❌ Don't**:
- Heuristic-based (may err)

---

### `visualfix` - Visualized Infix→Postfix
**Syntax**: `visualfix "<expression>"`

**Description**: Shows step-by-step infix to postfix conversion.

**Examples**:
```bash
visualfix "a+b*c"           # Shows stack operations
```

**✅ Do**:
- Learn Shunting Yard algorithm
- Understand operator precedence

**❌ Don't**:
- Educational only

---

### `visualpost` - Visualized Postfix→Infix
**Syntax**: `visualpost "<postfix>"`

**Description**: Shows step-by-step postfix to infix conversion.

**Examples**:
```bash
visualpost "a b + c *"      # Shows expression building
```

**✅ Do**:
- Learn stack-based conversion

**❌ Don't**:
- Educational only

---

### `visualeval` - Visualized Evaluation
**Syntax**: `visualeval "<postfix>"`

**Description**: Shows step-by-step postfix evaluation.

**Examples**:
```bash
visualeval "5 3 + 2 *"      # Shows calculation steps
```

**✅ Do**:
- Learn postfix evaluation
- See stack operations

**❌ Don't**:
- Educational only

---

## 10. Encoding/Decoding (4)

### `base64` - Base64 Encoding
**Syntax**: `base64 encode|decode "<text>"`

**Description**: Base64 encode/decode text.

**Examples**:
```bash
base64 encode "Hello"       # → SGVsbG8=
base64 decode "SGVsbG8="    # → Hello
```

**✅ Do**:
- Encode binary-safe data

**❌ Don't**:
- Not for encryption

---

### `morse` - Morse Code
**Syntax**: `morse encode|decode "<text>"`

**Description**: Convert to/from Morse code.

**Examples**:
```bash
morse encode "SOS"          # → ... --- ...
morse decode "... --- ..."  # → SOS
```

**✅ Do**:
- Fun encoding

**❌ Don't**:
- Limited charset

---

### `rot13` - ROT13 Cipher
**Syntax**: `rot13 "<text>"`

**Description**: Apply ROT13 cipher.

**Examples**:
```bash
rot13 "Hello"               # → Uryyb
rot13 "Uryyb"               # → Hello (reversible)
```

**✅ Do**:
- Simple obfuscation

**❌ Don't**:
- Not secure encryption

---

### `reverse` - Reverse Text
**Syntax**: `reverse "<text>"`

**Description**: Reverses a string.

**Examples**:
```bash
reverse "Hello"             # → olleH
```

**✅ Do**:
- String manipulation

**❌ Don't**:
- Unicode may not work

---

## 11. Number Conversions (4)

### `binary` - Decimal to Binary
**Syntax**: `binary <number>`

**Description**: Converts decimal to binary.

**Examples**:
```bash
binary 42                   # → 101010
binary 255                  # → 11111111
```

**✅ Do**:
- Learn binary representation

**❌ Don't**:
- Only positive integers

---

### `hex` - Decimal to Hexadecimal
**Syntax**: `hex <number>`

**Description**: Converts decimal to hexadecimal.

**Examples**:
```bash
hex 255                     # → 0xFF
hex 1234                    # → 0x4D2
```

**✅ Do**:
- Memory addresses

**❌ Don't**:
- Only positive integers

---

### `octal` - Decimal to Octal
**Syntax**: `octal <number>`

**Description**: Converts decimal to octal.

**Examples**:
```bash
octal 64                    # → 100
```

**✅ Do**:
- File permissions

**❌ Don't**:
- Only positive integers

---

### `baseconv` - Base Conversion
**Syntax**: `baseconv <number> <from_base> <to_base>`

**Description**: Converts between any bases (2-36).

**Examples**:
```bash
baseconv FF 16 10           # → 255
baseconv 101010 2 10        # → 42
baseconv 255 10 16          # → FF
```

**✅ Do**:
- Convert any base

**❌ Don't**:
- Bases must be 2-36

---

## 12. File Utilities (5)

### `checksum` - File Checksum
**Syntax**: `checksum <file>`

**Description**: Calculates simple file checksum.

**Examples**:
```bash
checksum file.txt           # → 0x4A2F
```

**✅ Do**:
- Verify file integrity

**❌ Don't**:
- Not cryptographic hash

---

### `crc32` - CRC32 Checksum
**Syntax**: `crc32 <file>`

**Description**: Calculates CRC32 checksum.

**Examples**:
```bash
crc32 file.txt              # → 0xAB12CD34
```

**✅ Do**:
- Error detection

**❌ Don't**:
- Not for security

---

### `compress` - Compress File
**Syntax**: `compress <file>`

**Description**: Simple RLE compression.

**Examples**:
```bash
compress file.txt           # Creates file.txt.compressed
```

**✅ Do**:
- Reduce file size

**❌ Don't**:
- Basic compression only

---

### `decompress` - Decompress File
**Syntax**: `decompress <file.compressed>`

**Description**: Decompresses file.

**Examples**:
```bash
decompress file.txt.compressed  # Restores file.txt
```

**✅ Do**:
- Restore compressed files

**❌ Don't**:
- Must match compress format

---

### `diff` - File Differences
**Syntax**: `diff <file1> <file2>`

**Description**: Shows differences between two files.

**Examples**:
```bash
diff old.txt new.txt        # Show differences
```

**✅ Do**:
- Compare file versions

**❌ Don't**:
- Basic line-by-line only

---

## 13. Generators (3)

### `random` - Random Number
**Syntax**: `random <min> <max>`

**Description**: Generates random number in range.

**Examples**:
```bash
random 1 100                # Random between 1-100
random 1 10                 # Random between 1-10
```

**✅ Do**:
- Testing
- Random selection

**❌ Don't**:
- Not cryptographically secure

---

### `uuid` - Generate UUID
**Syntax**: `uuid`

**Description**: Generates a UUID v4.

**Examples**:
```bash
uuid                        # → 550e8400-e29b-41d4-a716...
```

**✅ Do**:
- Unique identifiers

**❌ Don't**:
- Pseudo-random only

---

### `lorem` - Lorem Ipsum
**Syntax**: `lorem <words>`

**Description**: Generates Lorem Ipsum text.

**Examples**:
```bash
lorem 20                    # 20 words of Lorem Ipsum
lorem 50                    # 50 words
```

**✅ Do**:
- Placeholder text

**❌ Don't**:
- Limited word pool

---

## 14. Display Utilities (5)

### `colorpalette` - Color Grid
**Syntax**: `colorpalette`

**Description**: Shows 256-color terminal palette.

**Examples**:
```bash
colorpalette                # Show all colors
```

**✅ Do**:
- Test terminal colors

**❌ Don't**:
- Terminal must support 256 colors

---

### `ascii` - ASCII Art
**Syntax**: `ascii <text>`

**Description**: Converts text to ASCII art.

**Examples**:
```bash
ascii "HI"                  # ASCII art of HI
```

**✅ Do**:
- Fun text display

**❌ Don't**:
- Limited character set

---

### `progress` - Progress Bar
**Syntax**: `progress <percentage>`

**Description**: Shows progress bar.

**Examples**:
```bash
progress 25                 # 25% progress bar
progress 75                 # 75% progress bar
progress 100                # 100% with checkmark
```

**✅ Do**:
- Visual feedback

**❌ Don't**:
- 0-100 range only

---

### `table` - ASCII Table
**Syntax**: `table <rows> <cols>`

**Description**: Creates ASCII table.

**Examples**:
```bash
table 3 4                   # 3x4 table
```

**✅ Do**:
- Data formatting

**❌ Don't**:
- Empty cells

---

### `jsonformat` - Format JSON
**Syntax**: `jsonformat <file>`

**Description**: Pretty-prints JSON file.

**Examples**:
```bash
jsonformat config.json      # Format JSON
```

**✅ Do**:
- Readable JSON

**❌ Don't**:
- Basic implementation

---

## 15. Time Utilities (2)

### `timer` - Countdown Timer
**Syntax**: `timer <seconds>`

**Description**: Countdown timer.

**Examples**:
```bash
timer 10                    # Count down from 10
```

**✅ Do**:
- Timed operations

**❌ Don't**:
- Blocks terminal

---

### `stopwatch` - Stopwatch
**Syntax**: `stopwatch`

**Description**: Starts a stopwatch.

**Examples**:
```bash
stopwatch                   # Press Enter to stop
```

**✅ Do**:
- Measure duration

**❌ Don't**:
- Blocks until Enter

---

## 🎯 Best Practices Summary

### ✅ General Do's
1. Use quotes for expressions and text with spaces
2. Verify file existence before operations
3. Use absolute paths when in doubt
4. Back up important files before modifications
5. Check command output for errors
6. Use help and suggest to discover commands
7. Use visualizers to learn data structures
8. Test expressions with visualeval before using in scripts

### ❌ General Don'ts
1. Don't delete system files
2. Don't use on very large files without testing
3. Don't chain complex commands (limited support)
4. Don't expect all POSIX flags/options
5. Don't use for production encryption
6. Don't forget quotes on expressions
7. Don't run destructive commands without undo plan

---

**End of User Manual**  
*For more information, see README.md and DSAreport.md*
