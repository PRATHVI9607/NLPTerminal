/**
 * NLP Engine Implementation - Natural Language Processing for Shell Commands
 * Pure C implementation using pattern matching and keyword extraction
 * EXPANDED with 200+ natural language patterns
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "nlp_engine.h"

// ============ Pattern Definitions ============

typedef struct {
    const char *patterns[20];  // Multiple patterns that match
    int pattern_count;
    const char *command_template;
    const char *explanation;
} NLPPattern;

// All supported NLP patterns - MASSIVELY EXPANDED
static NLPPattern nlp_patterns[] = {
    // ═══════════════════════════════════════════════════════════════════
    //                    FILE LISTING & VIEWING
    // ═══════════════════════════════════════════════════════════════════
    
    {{"show files", "list files", "display files", "show all files", "list all files", 
      "what files", "see files", "view files", "files here", "whats here", 
      "show me files", "show me the files", "what files are here", "files in this folder",
      "what do i have here", "show directory contents", "dir contents", "folder contents"}, 18, 
     "ls", "Listing files in current directory"},
    
    {{"show everything", "list everything", "show all", "list all", "show contents",
      "what do we have", "show hidden", "show hidden files", "all files including hidden",
      "show me everything", "see everything", "view all files", "complete listing",
      "show all including hidden", "detailed listing", "full listing"}, 16,
     "ls -la", "Listing all files including hidden"},
    
    {{"just folders", "only folders", "list folders", "show folders", "directories only",
      "only directories", "list directories", "show directories", "what folders"}, 9,
     "ls -d */", "Listing only directories"},
    
    {{"sort by size", "biggest first", "largest first", "files by size", "order by size"}, 5,
     "ls -lS", "Listing files sorted by size"},
    
    {{"sort by time", "newest first", "recent first", "latest first", "by date"}, 5,
     "ls -lt", "Listing files sorted by time"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    DIRECTORY NAVIGATION
    // ═══════════════════════════════════════════════════════════════════
    
    {{"where am i", "current directory", "current path", "current location", 
      "show directory", "print directory", "what directory", "which folder",
      "current folder", "what folder am i in", "show my location", "my location",
      "present directory", "working directory", "show path"}, 15,
     "pwd", "Showing current working directory"},
    
    {{"go to", "change to", "navigate to", "switch to", "cd to", "enter folder", 
      "enter directory", "open folder", "move to", "jump to", "take me to",
      "bring me to", "go into", "change directory to", "switch directory to"}, 15,
     "cd %s", "Changing directory"},
    
    {{"go back", "go up", "parent directory", "go to parent", "up one level",
      "previous folder", "back up", "move up", "go one up", "one level up",
      "parent folder", "up a level", "back one", "folder above"}, 14,
     "cd ..", "Going to parent directory"},
    
    {{"go home", "home directory", "home folder", "go to home", "take me home",
      "my home", "user directory", "go to my folder", "back to home", "return home",
      "my folder", "personal folder", "user folder"}, 13,
     "cd ~", "Going to home directory"},
    
    {{"go root", "go to root", "root directory", "system root", "filesystem root",
      "top level", "go to top", "main directory"}, 8,
     "cd /", "Going to root directory"},
    
    {{"go previous", "go back to previous", "last directory", "previous directory",
      "where was i", "go to last folder", "return to previous"}, 7,
     "cd -", "Going to previous directory"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    DIRECTORY TREE
    // ═══════════════════════════════════════════════════════════════════
    
    {{"show tree", "display tree", "directory tree", "show directory tree", "folder structure",
      "show hierarchy", "directory structure", "folder tree", "tree view", "visualize folders",
      "show folder structure", "display hierarchy", "file tree"}, 13,
     "tree", "Displaying directory tree structure"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    CREATE FILES/FOLDERS
    // ═══════════════════════════════════════════════════════════════════
    
    {{"create folder", "make folder", "new folder", "create directory", 
      "make directory", "new directory", "add folder", "make a folder",
      "create a directory", "make a new folder", "create new folder",
      "i need a folder", "i want a folder", "add a directory"}, 14,
     "mkdir %s", "Creating new directory"},
    
    {{"create file", "make file", "new file", "create new file", "touch file",
      "add file", "make a file", "generate file", "create empty file",
      "make new file", "i need a file", "i want a file", "add a file"}, 13,
     "touch %s", "Creating new file"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    DELETE/REMOVE
    // ═══════════════════════════════════════════════════════════════════
    
    {{"delete file", "remove file", "erase file", "delete the file", "rm file",
      "kill file", "destroy file", "get rid of file", "eliminate file", "trash file",
      "i want to delete", "i need to remove", "please delete", "can you delete"}, 14,
     "rm %s", "Removing file"},
    
    {{"delete folder", "remove folder", "delete directory", "remove directory", 
      "erase folder", "kill folder", "destroy folder", "eliminate folder",
      "get rid of folder", "trash folder", "remove this folder"}, 11,
     "rmdir %s", "Removing directory"},
    
    {{"delete everything", "remove all", "delete all files", "remove everything",
      "clean up", "wipe everything", "erase all", "clear everything",
      "delete all", "remove all files"}, 10,
     "rm -rf *", "WARNING: Deleting all files"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    COPY/MOVE/RENAME
    // ═══════════════════════════════════════════════════════════════════
    
    {{"copy file", "duplicate file", "copy the file", "make copy of", "clone file",
      "replicate file", "make a copy", "copy this", "duplicate this",
      "i want to copy", "can you copy", "please copy"}, 12,
     "cp %s %s", "Copying file"},
    
    {{"backup this", "make backup", "backup the file", "save copy", "backup file",
      "create backup", "make a backup", "save a backup"}, 8,
     "cp %s %s.backup", "Creating backup copy"},
    
    {{"move file", "relocate file", "move the file", "transfer file", 
      "shift file", "relocate", "move this file", "i want to move"}, 8,
     "mv %s %s", "Moving file"},
    
    {{"rename file", "rename the file", "change name", "give new name", 
      "change file name", "rename this", "i want to rename"}, 7,
     "mv %s %s", "Renaming file"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    READ/VIEW FILE CONTENTS
    // ═══════════════════════════════════════════════════════════════════
    
    {{"read file", "show file", "display file", "print file", "view file", 
      "cat file", "show contents", "what is in", "whats in", "file contents",
      "open file", "show me file", "read contents", "display contents",
      "show file contents", "view contents", "read this file", "show this file"}, 18,
     "cat %s", "Displaying file contents"},
    
    {{"show beginning", "first lines", "show first", "head of file", "beginning of",
      "top of file", "start of file", "first part", "show start", "show top",
      "first 10 lines", "beginning lines", "top lines"}, 13,
     "head %s", "Showing first lines of file"},
    
    {{"show end", "last lines", "show last", "tail of file", "end of", 
      "bottom of file", "final lines", "ending of", "show ending", "show bottom",
      "last 10 lines", "bottom lines", "final part"}, 13,
     "tail %s", "Showing last lines of file"},
    
    {{"watch file", "monitor file", "follow file", "tail live", "watch changes",
      "follow changes", "monitor changes", "live view", "live tail",
      "watch for changes", "follow live", "real time view"}, 12,
     "tail -f %s", "Following file changes live"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    SEARCH/FIND
    // ═══════════════════════════════════════════════════════════════════
    
    {{"search for", "find text", "look for", "search text", "grep for",
      "locate text", "find string", "search pattern", "look up", "seek",
      "search in files", "find in files", "look for text", "search inside",
      "find word", "search word", "where is text"}, 17,
     "search %s", "Searching for pattern in files"},
    
    {{"find file", "locate file", "search file", "find filename", "where is file",
      "look for file", "search for file", "which file", "find a file named",
      "locate a file", "where is the file", "can you find file"}, 12,
     "find . -name %s", "Finding files by name"},
    
    {{"find duplicates", "duplicate files", "find duplicate", "show duplicates",
      "which duplicates", "repeated files", "same files", "identical files",
      "find repeated", "find same files", "duplicate finder"}, 11,
     "duplicate", "Finding duplicate files"},
    
    {{"find recent", "recent files", "show recent", "recently modified", "new files",
      "latest files", "newest files", "recently changed", "modified recently",
      "changed recently", "what was modified", "recent changes"}, 12,
     "recent", "Showing recently modified files"},
    
    {{"find large", "big files", "large files", "huge files", "biggest files",
      "largest files", "files by size", "what takes space", "space hogs",
      "heavy files", "fat files"}, 11,
     "du -sh * | sort -h", "Finding large files"},
    
    {{"find old", "old files", "ancient files", "aged files", "older than",
      "find outdated", "stale files", "old stuff"}, 8,
     "age %s older", "Finding old files"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    FILE ANALYSIS
    // ═══════════════════════════════════════════════════════════════════
    
    {{"compare files", "diff files", "check difference", "compare", "file diff",
      "show differences", "what changed", "differences between", "compare two files",
      "show diff", "find differences"}, 11,
     "compare %s %s", "Comparing two files"},
    
    {{"file info", "file details", "file information", "info about", "details of",
      "information on", "stats for", "metadata", "properties of", "file metadata",
      "tell me about file", "what is this file"}, 12,
     "fileinfo %s", "Showing detailed file information"},
    
    {{"hex view", "hexdump", "hex display", "binary view", "show hex",
      "hexadecimal view", "raw data", "binary dump", "show in hex",
      "view as hex", "hex dump"}, 11,
     "hexdump %s", "Showing hexadecimal dump"},
    
    {{"count words", "word count", "count lines", "line count", "wc",
      "how many words", "how many lines", "file statistics", "count characters",
      "character count", "statistics of"}, 11,
     "wc %s", "Counting words/lines in file"},
    
    {{"file size", "how big", "size of", "check size", "show size",
      "file dimensions", "space used", "how large", "what size",
      "tell me size", "size of file"}, 11,
     "sizeof %s", "Showing file/directory size"},
    
    {{"word frequency", "word freq", "count occurrences", "word stats",
      "most common words", "frequent words", "word analysis", "analyze words",
      "top words"}, 9,
     "freq %s", "Analyzing word frequency"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    ENCRYPTION
    // ═══════════════════════════════════════════════════════════════════
    
    {{"encrypt file", "secure file", "lock file", "protect file", "encode file",
      "cipher file", "encrypt this", "make secure", "password protect",
      "i want to encrypt", "please encrypt", "encrypt my file"}, 12,
     "encrypt %s %s", "Encrypting file with key"},
    
    {{"decrypt file", "unlock file", "decode file", "decipher file", "decrypt this",
      "unsecure file", "unlock this", "unprotect file", "i want to decrypt",
      "please decrypt", "decrypt my file"}, 11,
     "decrypt %s %s", "Decrypting file with key"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    SYSTEM MONITORING
    // ═══════════════════════════════════════════════════════════════════
    
    {{"system monitor", "show system", "system info", "system status", 
      "resource monitor", "show resources", "system stats", "resource usage",
      "performance monitor", "system resources", "monitor system",
      "how is my system", "system health", "check system", "system overview"}, 15,
     "sysmon", "Opening system resource monitor"},
    
    {{"cpu usage", "cpu info", "processor info", "show cpu", "cpu status",
      "how much cpu", "processor usage", "cpu stats", "check cpu",
      "cpu load", "processor status"}, 11,
     "cpuinfo", "Showing CPU information"},
    
    {{"memory usage", "ram usage", "memory stats", "free memory", "used memory",
      "how much ram", "memory info", "mem stats", "show memory", "check memory",
      "available memory", "memory status", "ram info"}, 13,
     "meminfo", "Showing memory usage"},
    
    {{"disk usage", "disk space", "free space", "storage space", "storage info",
      "how much space", "available space", "storage left", "disk stats",
      "disk info", "check disk", "storage status", "hard drive space"}, 13,
     "diskinfo", "Showing disk space usage"},
    
    {{"process list", "running processes", "list processes", "show processes", 
      "active processes", "what's running", "current processes", "tasks running",
      "all processes", "show tasks", "running tasks", "background processes"}, 12,
     "proclist", "Listing running processes"},
    
    {{"top processes", "heaviest processes", "resource hogs", "cpu hogs",
      "what's using cpu", "what's using memory", "process ranking", "top tasks"}, 8,
     "proctop", "Showing top resource-consuming processes"},
    
    {{"network info", "network stats", "network status", "connections",
      "show connections", "network connections", "active connections",
      "internet connections", "tcp connections", "open connections"}, 10,
     "netinfo", "Showing network information"},
    
    {{"load average", "system load", "cpu load", "show load", "current load"}, 5,
     "loadavg", "Showing system load average"},
    
    {{"swap info", "swap usage", "swap space", "virtual memory", "show swap"}, 5,
     "swapinfo", "Showing swap usage"},
    
    {{"paging stats", "page faults", "paging info", "memory paging"}, 4,
     "paging", "Showing paging statistics"},
    
    {{"vmstat", "virtual memory stats", "vm stats", "memory statistics"}, 4,
     "vmstat", "Showing virtual memory statistics"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    DATE/TIME/USER
    // ═══════════════════════════════════════════════════════════════════
    
    {{"current time", "what time", "show time", "current date", "show date", 
      "date and time", "time now", "todays date", "what day", "when is it",
      "what's the time", "what's the date", "tell me the time", "tell me date"}, 14,
     "date", "Showing current date and time"},
    
    {{"who am i", "current user", "my username", "logged in as", "my login",
      "username", "user info", "which user", "what user am i",
      "show my username", "what's my username"}, 11,
     "whoami", "Showing current user"},
    
    {{"hostname", "computer name", "machine name", "system name", "host",
      "this computer", "pc name", "what's my hostname", "show hostname"}, 9,
     "hostname", "Showing hostname"},
    
    {{"uptime", "how long running", "system uptime", "running time", "been up",
      "system time", "running since", "how long has system been up"}, 8,
     "uptime", "Showing system uptime"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    PROCESS MANAGEMENT
    // ═══════════════════════════════════════════════════════════════════
    
    {{"kill process", "stop process", "end process", "terminate process",
      "force stop", "end task", "stop task", "kill task", "terminate task",
      "i want to kill", "please stop process", "shut down process"}, 12,
     "prockill %s", "Killing process"},
    
    {{"process info", "process details", "show process", "process stats",
      "info about process", "process information", "details of process"}, 7,
     "proc %s", "Showing process information"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    COMPRESSION
    // ═══════════════════════════════════════════════════════════════════
    
    {{"compress file", "zip file", "compress this", "make smaller", "compact file",
      "archive file", "pack file", "squeeze file", "i want to compress",
      "please compress", "shrink file"}, 11,
     "compress %s", "Compressing file using RLE"},
    
    {{"decompress file", "unzip file", "extract file", "uncompress", "unpack file",
      "expand file", "restore file", "unsqueeze", "i want to decompress",
      "please decompress", "extract archive"}, 11,
     "decompress %s", "Decompressing RLE file"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    ENCODING/DECODING
    // ═══════════════════════════════════════════════════════════════════
    
    {{"encode base64", "base64 encode", "to base64", "base64 this", "convert to base64",
      "make base64", "base64 encoding"}, 7,
     "base64 encode %s", "Encoding text to Base64"},
    
    {{"decode base64", "base64 decode", "from base64", "unbase64", "convert from base64"}, 5,
     "base64 decode %s", "Decoding Base64 text"},
    
    {{"morse code", "to morse", "encode morse", "morse encode", "convert to morse",
      "make morse code"}, 6,
     "morse encode %s", "Converting to Morse code"},
    
    {{"decode morse", "from morse", "morse decode", "convert from morse"}, 4,
     "morse decode %s", "Decoding Morse code"},
    
    {{"rot13", "caesar cipher", "rotate 13", "rot 13", "simple cipher"}, 5,
     "rot13 %s", "Applying ROT13 cipher"},
    
    {{"reverse text", "backwards text", "flip text", "mirror text", "text backwards"}, 5,
     "reverse %s", "Reversing text"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    NUMBER CONVERSIONS
    // ═══════════════════════════════════════════════════════════════════
    
    {{"to binary", "convert to binary", "binary of", "show binary", "in binary",
      "what is in binary", "binary conversion", "make binary"}, 8,
     "binary %s", "Converting to binary"},
    
    {{"to hex", "to hexadecimal", "convert to hex", "hex of", "in hex",
      "what is in hex", "hexadecimal of", "make hex"}, 8,
     "hex %s", "Converting to hexadecimal"},
    
    {{"to octal", "convert to octal", "octal of", "in octal", "what is in octal"}, 5,
     "octal %s", "Converting to octal"},
    
    {{"convert base", "base conversion", "change base", "number base",
      "convert number base"}, 5,
     "baseconv %s", "Converting between number bases"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    MATH & CALCULATOR
    // ═══════════════════════════════════════════════════════════════════
    
    {{"calculate", "calc", "compute", "math", "solve", "do math", "work out",
      "figure out", "what is", "whats", "how much is", "tell me result",
      "evaluate math", "calculator"}, 14,
     "calc %s", "Calculating mathematical expression"},
    
    {{"random number", "generate random", "random", "rand", "give me random",
      "pick random", "random integer", "generate a number"}, 8,
     "random %s", "Generating random number"},
    
    {{"generate uuid", "uuid", "unique id", "guid", "make uuid", "create uuid"}, 6,
     "uuid", "Generating UUID"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    EXPRESSION CONVERSIONS
    // ═══════════════════════════════════════════════════════════════════
    
    {{"infix to postfix", "convert infix", "to postfix", "postfix notation",
      "make postfix", "infix conversion", "convert to postfix"}, 7,
     "infix2postfix %s", "Converting infix to postfix notation"},
    
    {{"infix to prefix", "to prefix", "prefix notation", "make prefix",
      "convert to prefix"}, 5,
     "infix2prefix %s", "Converting infix to prefix notation"},
    
    {{"postfix to infix", "from postfix", "postfix convert", "convert postfix",
      "postfix to normal"}, 5,
     "postfix2infix %s", "Converting postfix to infix notation"},
    
    {{"prefix to infix", "from prefix", "prefix convert", "convert prefix",
      "prefix to normal"}, 5,
     "prefix2infix %s", "Converting prefix to infix notation"},
    
    {{"evaluate expression", "eval expression", "evaluate postfix", "compute postfix",
      "solve expression", "evaluate this expression"}, 6,
     "evaluate %s", "Evaluating postfix expression"},
    
    {{"expression type", "detect expression", "what type expression",
      "identify expression", "which notation"}, 5,
     "exprtype %s", "Detecting expression type"},
    
    {{"visualize infix", "show infix conversion", "visual infix", "visualfix",
      "see infix conversion", "animate infix"}, 6,
     "visualfix %s", "Visualizing infix to postfix conversion"},
    
    {{"visualize postfix", "show postfix conversion", "visual postfix",
      "see postfix conversion"}, 4,
     "visualpost %s", "Visualizing postfix to infix conversion"},
    
    {{"visualize evaluation", "visual eval", "step by step eval", "show evaluation",
      "visualeval", "visualize eval", "visual evaluation", "show steps",
      "step by step", "show work", "demonstrate", "explain steps",
      "trace evaluation", "evaluation trace"}, 14,
     "visualeval %s", "Visualizing postfix evaluation step-by-step"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    DATA STRUCTURE VISUALIZATIONS
    // ═══════════════════════════════════════════════════════════════════
    
    {{"visualize history", "visual history", "show history visual", "history graph"}, 4,
     "history", "Visualizing command history"},
    
    {{"visualize stack", "show stack", "display stack", "visual stack",
      "stack visualization", "see stack"}, 6,
     "visualize stack", "Visualizing stack data structure"},
    
    {{"visualize list", "visualize linked list", "show list", "visual list",
      "linked list visual", "see linked list"}, 6,
     "visualize list", "Visualizing linked list"},
    
    {{"visualize tree", "visualize binary tree", "show tree visual", "visual tree",
      "tree visualization", "see tree structure"}, 6,
     "visualize tree", "Visualizing tree data structure"},
    
    {{"visualize trie", "show trie", "visual trie", "trie visualization"}, 4,
     "visualize trie", "Visualizing trie data structure"},
    
    {{"visualize undo", "show undo stack", "undo stack", "undo visualization"}, 4,
     "visualize undo", "Visualizing undo stack"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    NOTES/BOOKMARKS
    // ═══════════════════════════════════════════════════════════════════
    
    {{"add note", "quick note", "save note", "take note", "note this",
      "remember this", "jot down", "write note", "make a note",
      "i want to note", "new note"}, 11,
     "quicknote add %s", "Adding a quick note"},
    
    {{"show notes", "list notes", "my notes", "view notes", "all notes",
      "saved notes", "read notes", "see my notes", "what notes do i have"}, 9,
     "quicknote list", "Showing saved notes"},
    
    {{"delete note", "remove note", "erase note", "clear note"}, 4,
     "quicknote delete %s", "Deleting a note"},
    
    {{"search notes", "find note", "look for note", "search in notes"}, 4,
     "quicknote search %s", "Searching notes"},
    
    {{"bookmark", "save bookmark", "mark this", "bookmark this", "save location",
      "remember location", "mark location"}, 7,
     "bookmark %s", "Saving bookmark"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    TEXT UTILITIES
    // ═══════════════════════════════════════════════════════════════════
    
    {{"sort file", "sort lines", "order lines", "arrange lines", "sort this",
      "alphabetize", "sort alphabetically"}, 7,
     "sort %s", "Sorting file contents"},
    
    {{"unique lines", "remove duplicates", "distinct lines", "no duplicates",
      "deduplicate", "unique only"}, 6,
     "uniq %s", "Showing unique lines"},
    
    {{"format json", "pretty json", "json format", "beautify json", "json pretty",
      "prettify json", "indent json"}, 7,
     "jsonformat %s", "Formatting JSON"},
    
    {{"show json", "cat json", "view json", "display json", "read json"}, 5,
     "jsoncat %s", "Displaying formatted JSON"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    DISPLAY/VISUAL
    // ═══════════════════════════════════════════════════════════════════
    
    {{"show table", "display table", "table view", "make table", "format table",
      "as table", "table format"}, 7,
     "table %s", "Displaying as table"},
    
    {{"show colors", "color palette", "colors", "palette", "color codes",
      "available colors", "all colors"}, 7,
     "colorpalette", "Showing color palette"},
    
    {{"show progress", "progress bar", "loading bar", "show percentage"}, 4,
     "progress %s", "Showing progress indicator"},
    
    {{"ascii art", "ascii", "show ascii", "text art", "make ascii"}, 5,
     "ascii %s", "Displaying ASCII art"},
    
    {{"banner", "big text", "large text", "figlet text", "ascii banner",
      "make banner", "text banner"}, 7,
     "banner %s", "Creating ASCII banner"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    FILE UTILITIES
    // ═══════════════════════════════════════════════════════════════════
    
    {{"checksum", "file checksum", "calculate checksum", "hash file", "file hash",
      "verify checksum", "get hash"}, 7,
     "checksum %s", "Calculating file checksum"},
    
    {{"crc32", "crc check", "crc of file", "check crc", "calculate crc"}, 5,
     "crc32 %s", "Calculating CRC32 checksum"},
    
    {{"lorem ipsum", "sample text", "placeholder text", "dummy text",
      "generate text", "filler text"}, 6,
     "lorem %s", "Generating Lorem Ipsum text"},
    
    {{"timer", "set timer", "countdown", "start timer", "countdown timer"}, 5,
     "timer %s", "Starting countdown timer"},
    
    {{"stopwatch", "start stopwatch", "time this", "measure time"}, 4,
     "stopwatch", "Starting stopwatch"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    SCREEN/TERMINAL
    // ═══════════════════════════════════════════════════════════════════
    
    {{"clear screen", "clear terminal", "cls", "clear", "clean screen",
      "clear display", "reset screen", "blank screen", "wipe screen",
      "clean terminal", "fresh screen"}, 11,
     "clear", "Clearing the screen"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    HISTORY/UNDO/MACROS
    // ═══════════════════════════════════════════════════════════════════
    
    {{"show history", "command history", "previous commands", "history",
      "past commands", "command log", "what did i do", "recent commands",
      "my history", "show my history", "list history"}, 11,
     "history", "Showing command history"},
    
    {{"undo", "undo last", "reverse last", "undo that", "revert",
      "undo command", "take back", "undo previous", "go back one",
      "undo my last command"}, 10,
     "undo", "Undoing last operation"},
    
    {{"create macro", "make macro", "define macro", "new macro", "record macro",
      "save macro", "macro define"}, 7,
     "macro define %s", "Defining a new macro"},
    
    {{"run macro", "execute macro", "play macro", "use macro"}, 4,
     "macro run %s", "Running a macro"},
    
    {{"list macros", "show macros", "my macros", "all macros", "available macros"}, 5,
     "macro list", "Listing all macros"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    HELP/INFO
    // ═══════════════════════════════════════════════════════════════════
    
    {{"help", "show help", "help me", "what commands", "available commands", 
      "show commands", "list commands", "what can you do", "how to use",
      "instructions", "manual", "guide", "assist me", "i need help",
      "please help", "can you help", "show me commands"}, 17,
     "help", "Showing available commands"},
    
    {{"help with", "how to", "how do i", "explain", "tell me about",
      "info on", "information on", "what is", "describe", "teach me",
      "show me how to", "i want to know about"}, 12,
     "help %s", "Showing help for command"},
    
    {{"show categories", "command categories", "list categories", "what categories",
      "categories", "command groups"}, 6,
     "categories", "Showing command categories"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    IPC/MESSAGING
    // ═══════════════════════════════════════════════════════════════════
    
    {{"my terminals", "list terminals", "show terminals", "other terminals",
      "terminal list", "all terminals"}, 6,
     "terminals", "Listing terminal sessions"},
    
    {{"send message", "message terminal", "send to terminal", "broadcast message"}, 4,
     "sendmsg %s", "Sending message to terminal"},
    
    {{"check messages", "any messages", "show messages", "read messages",
      "do i have messages", "unread messages"}, 6,
     "checkmsg", "Checking for messages"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    CONVERSATIONAL/POLITE FORMS
    // ═══════════════════════════════════════════════════════════════════
    
    {{"can you show me files", "could you list files", "would you show files",
      "please show files", "i'd like to see files", "let me see files"}, 6,
     "ls", "Listing files in current directory"},
    
    {{"can you tell me where i am", "could you show my location",
      "where am i please", "show me current directory please"}, 4,
     "pwd", "Showing current working directory"},
    
    {{"i want to go to", "take me to", "bring me to", "i need to go to",
      "can we go to", "lets go to"}, 6,
     "cd %s", "Changing directory"},
    
    {{"can you create", "could you make", "please create", "i want you to create",
      "would you create", "make me a"}, 6,
     "mkdir %s", "Creating new directory"},
    
    {{"can you delete", "could you remove", "please delete", "i want you to delete",
      "would you remove", "get rid of"}, 6,
     "rm %s", "Removing file"},
    
    {{"can you show system", "how is the system doing", "system health please",
      "i want to see system status", "show me system info please"}, 5,
     "sysmon", "Opening system resource monitor"},
    
    // ═══════════════════════════════════════════════════════════════════
    //                    EXIT
    // ═══════════════════════════════════════════════════════════════════
    
    {{"exit", "quit", "close", "bye", "goodbye", "leave", "logout",
      "sign out", "im done", "thats all", "terminate", "end session",
      "close terminal", "exit terminal", "i want to exit", "close shell"}, 16,
     "exit", "Exiting the shell"},
};
    

static int num_patterns = sizeof(nlp_patterns) / sizeof(nlp_patterns[0]);

// ============ Available Commands List (EXPANDED) ============

static const char *available_commands[] = {
    // Basic File Operations
    "ls", "dir", "pwd", "cd", "mkdir", "rmdir", "touch", "rm", "cat", "cp", "mv", "echo",
    // Advanced File Operations  
    "tree", "search", "grep", "backup", "compare", "diff", "fileinfo", "hexdump", "duplicate",
    // File Analysis
    "sizeof", "age", "freq", "lines", "wc", "checksum", "crc32", "head", "tail",
    // Encryption
    "encrypt", "decrypt",
    // System Monitoring
    "sysmon", "cpuinfo", "meminfo", "diskinfo", "proclist", "proctop", "proc",
    "netinfo", "netstat", "connections", "loadavg", "swapinfo", "paging", "vmstat",
    "kernelinfo", "filesystems", "mounts", "modules", "battery", "sensors",
    "interrupts", "users", "envvar", "openfiles", "sockets", "zoneinfo",
    // Date/Time/User
    "date", "whoami", "hostname", "uptime",
    // Process Management
    "ps", "kill", "prockill",
    // Compression
    "compress", "decompress",
    // Encoding/Decoding
    "base64", "morse", "rot13", "reverse",
    // Number Conversions
    "binary", "hex", "octal", "baseconv",
    // Expression/Math
    "calc", "random", "uuid",
    "infix2postfix", "infix2prefix", "postfix2infix", "prefix2infix",
    "evaluate", "exprtype", "visualfix", "visualpost", "visualeval",
    // Visualization
    "visualize", "colorpalette", "ascii", "progress", "table", "banner",
    // Text Processing
    "sort", "uniq", "jsonformat", "jsoncat",
    // Utilities
    "timer", "stopwatch", "lorem",
    // Notes/Bookmarks
    "quicknote", "bookmark",
    // Shell Features
    "history", "recent", "bulk_rename", "stats", "undo", "macro", "teach",
    // IPC
    "terminals", "sendmsg", "broadcast", "checkmsg", "readmsg",
    // Help/Exit
    "clear", "help", "categories", "exit"
};
static int num_commands = sizeof(available_commands) / sizeof(available_commands[0]);

// ============ Helper Functions ============

// Convert string to lowercase
static void str_to_lower(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

// Trim whitespace from both ends
static char* str_trim(char *str) {
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    
    return str;
}

// Check if string contains substring
static int str_contains(const char *haystack, const char *needle) {
    return strstr(haystack, needle) != NULL;
}

// Extract argument after a keyword (e.g., "called test" returns "test")
static int extract_argument(const char *input, const char *keywords[], int keyword_count, char *output) {
    char temp[MAX_PATTERN_LEN];
    strncpy(temp, input, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';
    str_to_lower(temp);
    
    for (int i = 0; i < keyword_count; i++) {
        char *pos = strstr(temp, keywords[i]);
        if (pos) {
            pos += strlen(keywords[i]);
            while (*pos && isspace((unsigned char)*pos)) pos++;
            
            // Find end of argument
            char *end = pos;
            while (*end && !isspace((unsigned char)*end)) end++;
            
            int len = end - pos;
            if (len > 0) {
                // Get from original string to preserve case
                int offset = pos - temp;
                strncpy(output, input + offset, len);
                output[len] = '\0';
                return 1;
            }
        }
    }
    return 0;
}

// Extract everything after a pattern (for expressions with spaces)
static int extract_rest_after_pattern(const char *input, const char *pattern, char *output) {
    char temp[MAX_PATTERN_LEN];
    strncpy(temp, input, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';
    str_to_lower(temp);
    
    char *pos = strstr(temp, pattern);
    if (pos) {
        pos += strlen(pattern);
        while (*pos && isspace((unsigned char)*pos)) pos++;
        
        if (*pos) {
            // Get from original string to preserve case
            int offset = pos - temp;
            strcpy(output, input + offset);
            // Trim trailing whitespace
            int len = strlen(output);
            while (len > 0 && isspace((unsigned char)output[len-1])) {
                output[--len] = '\0';
            }
            return strlen(output) > 0;
        }
    }
    return 0;
}

// Extract last word from string
static int extract_last_word(const char *input, char *output) {
    const char *last_space = strrchr(input, ' ');
    if (last_space) {
        strcpy(output, last_space + 1);
        return strlen(output) > 0;
    }
    return 0;
}

// Extract words between two keywords
static int extract_between(const char *input, const char *from_kw __attribute__((unused)), const char *to_kw, char *arg1, char *arg2) {
    char temp[MAX_PATTERN_LEN];
    strncpy(temp, input, sizeof(temp) - 1);
    str_to_lower(temp);
    
    char *to_pos = strstr(temp, to_kw);
    if (!to_pos) return 0;
    
    // Get second argument (after "to")
    char *arg2_start = to_pos + strlen(to_kw);
    while (*arg2_start && isspace((unsigned char)*arg2_start)) arg2_start++;
    
    char *arg2_end = arg2_start;
    while (*arg2_end && !isspace((unsigned char)*arg2_end)) arg2_end++;
    
    int offset2 = arg2_start - temp;
    int len2 = arg2_end - arg2_start;
    strncpy(arg2, input + offset2, len2);
    arg2[len2] = '\0';
    
    // Get first argument (before "to", after keywords like "copy", "move", etc.)
    *to_pos = '\0';  // Temporarily terminate
    
    // Find last significant word before "to"
    char *word_end = to_pos - 1;
    while (word_end > temp && isspace((unsigned char)*word_end)) word_end--;
    
    char *word_start = word_end;
    while (word_start > temp && !isspace((unsigned char)*(word_start - 1))) word_start--;
    
    int offset1 = word_start - temp;
    int len1 = word_end - word_start + 1;
    strncpy(arg1, input + offset1, len1);
    arg1[len1] = '\0';
    
    return strlen(arg1) > 0 && strlen(arg2) > 0;
}

// ============ Main NLP Functions ============

void nlp_init(void) {
    // Initialization if needed (currently stateless)
}

NLPResult nlp_translate(const char *input) {
    NLPResult result;
    memset(&result, 0, sizeof(result));
    strncpy(result.original, input, sizeof(result.original) - 1);
    strncpy(result.translated, input, sizeof(result.translated) - 1);
    result.was_translated = 0;
    
    if (!input || strlen(input) == 0) {
        return result;
    }
    
    // Normalize input
    char normalized[MAX_PATTERN_LEN];
    strncpy(normalized, input, sizeof(normalized) - 1);
    normalized[sizeof(normalized) - 1] = '\0';
    str_to_lower(normalized);
    char *trimmed = str_trim(normalized);
    
    // Skip NLP translation for built-in commands that take arguments
    // These should NOT be NLP translated (help, man, etc.)
    const char *skip_commands[] = {
        "oshelp", "ipchelp", "uniquecmds", "mypid", "terminals", "sendmsg ", "broadcast ",
        "checkmsg", "readmsg", "clearmsg", "register", "unregister",
        "help ", "man ", "info ", "which ", "type ",
        "ls ", "cd ", "cat ", "mkdir ", "rmdir ", "rm ", "cp ", "mv ",
        "touch ", "echo ", "grep ", "head ", "tail ", "wc ",
        "chmod ", "chown ", "visualeval ", "evaluate ", "calc ",
        "infix2postfix ", "infix2prefix ", "postfix2infix ", "prefix2infix ",
        "visualfix ", "visualpost ", "compress ", "decompress ",
        "encrypt ", "decrypt ", "base64 ", "binary ", "hex ", "octal ",
        "paging", "vmstat", "meminfo", "cpuinfo", "proclist", "proctop",
        "netinfo", "netstat", "connections", "loadavg", "kernelinfo",
        "filesystems", "mounts", "modules", "battery", "sensors",
        "interrupts", "users", "envvar", "openfiles", "sockets",
        "diskinfo", "swapinfo", "zoneinfo", "prockill",
        NULL
    };
    
    for (int i = 0; skip_commands[i] != NULL; i++) {
        if (strncmp(trimmed, skip_commands[i], strlen(skip_commands[i])) == 0) {
            // This is a direct command with arguments, don't NLP translate
            return result;
        }
    }
    
    // Check each pattern
    for (int i = 0; i < num_patterns; i++) {
        NLPPattern *p = &nlp_patterns[i];
        
        for (int j = 0; j < p->pattern_count; j++) {
            // Match if pattern is substring of input OR if input starts with pattern words
            if (str_contains(trimmed, p->patterns[j]) || 
                strncmp(trimmed, p->patterns[j], strlen(p->patterns[j])) == 0) {
                // Found a match!
                strncpy(result.explanation, p->explanation, sizeof(result.explanation) - 1);
                
                // Check if command needs arguments
                if (strstr(p->command_template, "%s")) {
                    // Needs argument extraction
                    const char *arg_keywords[] = {"called", "named", "file", "folder", "directory", "to"};
                    char arg1[256] = "", arg2[256] = "";
                    
                    // Check for two-argument commands (copy, move, compare)
                    if (strstr(p->command_template, "%s %s")) {
                        if (extract_between(input, "", " to ", arg1, arg2) ||
                            extract_between(input, "", " and ", arg1, arg2) ||
                            extract_between(input, "", " with ", arg1, arg2)) {
                            snprintf(result.translated, sizeof(result.translated), 
                                    p->command_template, arg1, arg2);
                            result.was_translated = 1;
                            return result;
                        }
                    }
                    
                    // For expression commands (visualeval, evaluate, calc, etc.), 
                    // extract everything after the pattern as the expression
                    if (strstr(p->command_template, "visualeval") ||
                        strstr(p->command_template, "evaluate") ||
                        strstr(p->command_template, "calc") ||
                        strstr(p->command_template, "infix2postfix") ||
                        strstr(p->command_template, "infix2prefix") ||
                        strstr(p->command_template, "postfix2infix") ||
                        strstr(p->command_template, "prefix2infix") ||
                        strstr(p->command_template, "visualfix") ||
                        strstr(p->command_template, "visualpost")) {
                        // Try to extract rest of string after the matched pattern
                        if (extract_rest_after_pattern(input, p->patterns[j], arg1)) {
                            snprintf(result.translated, sizeof(result.translated),
                                    p->command_template, arg1);
                            result.was_translated = 1;
                            return result;
                        }
                    }
                    
                    // Single argument for other commands
                    if (extract_argument(input, arg_keywords, 6, arg1) ||
                        extract_last_word(input, arg1)) {
                        snprintf(result.translated, sizeof(result.translated),
                                p->command_template, arg1);
                        result.was_translated = 1;
                        return result;
                    }
                } else {
                    // No arguments needed
                    strncpy(result.translated, p->command_template, sizeof(result.translated) - 1);
                    result.was_translated = 1;
                    return result;
                }
            }
        }
    }
    
    // No translation found, return original
    return result;
}

void nlp_get_suggestions(const char *partial, SuggestionList *suggestions) {
    if (!suggestions) return;
    
    suggestions->count = 0;
    suggestions->selected_index = 0;
    
    if (!partial || strlen(partial) == 0) {
        return;
    }
    
    char lower_partial[256];
    strncpy(lower_partial, partial, sizeof(lower_partial) - 1);
    lower_partial[sizeof(lower_partial) - 1] = '\0';
    str_to_lower(lower_partial);
    
    int partial_len = strlen(lower_partial);
    
    // First, exact prefix matches (highest priority)
    for (int i = 0; i < num_commands && suggestions->count < MAX_SUGGESTIONS; i++) {
        if (strncmp(available_commands[i], lower_partial, partial_len) == 0) {
            strncpy(suggestions->suggestions[suggestions->count], 
                   available_commands[i], MAX_SUGGESTION_LEN - 1);
            suggestions->count++;
        }
    }
    
    // Then, substring matches (if not enough suggestions)
    if (suggestions->count < 3) {
        for (int i = 0; i < num_commands && suggestions->count < MAX_SUGGESTIONS; i++) {
            if (strstr(available_commands[i], lower_partial) != NULL) {
                // Check if already added
                int already_added = 0;
                for (int j = 0; j < suggestions->count; j++) {
                    if (strcmp(suggestions->suggestions[j], available_commands[i]) == 0) {
                        already_added = 1;
                        break;
                    }
                }
                if (!already_added) {
                    strncpy(suggestions->suggestions[suggestions->count],
                           available_commands[i], MAX_SUGGESTION_LEN - 1);
                    suggestions->count++;
                }
            }
        }
    }
    
    // Check for natural language patterns
    for (int i = 0; i < num_patterns && suggestions->count < MAX_SUGGESTIONS; i++) {
        for (int j = 0; j < nlp_patterns[i].pattern_count; j++) {
            if (str_contains(nlp_patterns[i].patterns[j], lower_partial) ||
                str_contains(lower_partial, nlp_patterns[i].patterns[j])) {
                // Extract base command from template
                char cmd[64];
                sscanf(nlp_patterns[i].command_template, "%63s", cmd);
                
                // Check if already added
                int already_added = 0;
                for (int k = 0; k < suggestions->count; k++) {
                    if (strcmp(suggestions->suggestions[k], cmd) == 0) {
                        already_added = 1;
                        break;
                    }
                }
                if (!already_added) {
                    strncpy(suggestions->suggestions[suggestions->count],
                           cmd, MAX_SUGGESTION_LEN - 1);
                    suggestions->count++;
                }
                break;
            }
        }
    }
}

const char* nlp_get_best_suggestion(const char *partial) {
    static char best[MAX_SUGGESTION_LEN];
    SuggestionList suggestions;
    
    nlp_get_suggestions(partial, &suggestions);
    
    if (suggestions.count > 0) {
        strncpy(best, suggestions.suggestions[0], sizeof(best) - 1);
        best[sizeof(best) - 1] = '\0';
        return best;
    }
    
    return NULL;
}

int nlp_is_natural_language(const char *input) {
    if (!input) return 0;
    
    // If it starts with a known command, it's not NL
    char first_word[64];
    sscanf(input, "%63s", first_word);
    str_to_lower(first_word);
    
    for (int i = 0; i < num_commands; i++) {
        if (strcmp(first_word, available_commands[i]) == 0) {
            return 0;
        }
    }
    
    // Check for NL indicators
    const char *nl_indicators[] = {
        "show", "list", "display", "create", "make", "delete", "remove",
        "what", "where", "how", "find", "search", "go", "help", "please",
        "can you", "i want", "i need", "system", "monitor", "visualize",
        "visual", "evaluate", "convert", "encode", "decode", "compress",
        "decompress", "encrypt", "decrypt", "backup", "compare", "current",
        "my", "who", "calculate", "calc", "random", "generate"
    };
    
    char lower[MAX_PATTERN_LEN];
    strncpy(lower, input, sizeof(lower) - 1);
    str_to_lower(lower);
    
    for (int i = 0; i < (int)(sizeof(nl_indicators) / sizeof(nl_indicators[0])); i++) {
        if (str_contains(lower, nl_indicators[i])) {
            return 1;
        }
    }
    
    // If contains multiple words and spaces, likely NL
    int spaces = 0;
    for (int i = 0; input[i]; i++) {
        if (input[i] == ' ') spaces++;
    }
    
    return spaces >= 2;
}

const char* nlp_get_command_help(const char *cmd) {
    static char help_text[512];
    
    if (strcmp(cmd, "ls") == 0) {
        return "ls [path] - List directory contents. Shows files and directories with their sizes.";
    } else if (strcmp(cmd, "pwd") == 0) {
        return "pwd - Print working directory. Shows the current directory path.";
    } else if (strcmp(cmd, "cd") == 0) {
        return "cd <path> - Change directory. Use '..' to go up, '~' for home.";
    } else if (strcmp(cmd, "mkdir") == 0) {
        return "mkdir <name> - Create a new directory.";
    } else if (strcmp(cmd, "rmdir") == 0) {
        return "rmdir <name> - Remove an empty directory.";
    } else if (strcmp(cmd, "touch") == 0) {
        return "touch <file> - Create a new empty file or update timestamp.";
    } else if (strcmp(cmd, "rm") == 0) {
        return "rm <file> - Remove/delete a file.";
    } else if (strcmp(cmd, "cat") == 0) {
        return "cat <file> - Display file contents.";
    } else if (strcmp(cmd, "cp") == 0) {
        return "cp <source> <dest> - Copy a file.";
    } else if (strcmp(cmd, "mv") == 0) {
        return "mv <source> <dest> - Move or rename a file.";
    } else if (strcmp(cmd, "sysmon") == 0) {
        return "sysmon [-l] - System resource monitor. Use -l for live mode.";
    } else if (strcmp(cmd, "tree") == 0) {
        return "tree [path] - Display directory structure as a tree.";
    } else if (strcmp(cmd, "search") == 0) {
        return "search <pattern> - Search for pattern in files.";
    } else if (strcmp(cmd, "fileinfo") == 0) {
        return "fileinfo <file> - Show detailed file information.";
    } else if (strcmp(cmd, "duplicate") == 0) {
        return "duplicate [path] - Find duplicate files by content.";
    } else if (strcmp(cmd, "hexdump") == 0) {
        return "hexdump <file> [offset] [len] - Display file in hexadecimal.";
    } else if (strcmp(cmd, "calc") == 0) {
        return "calc <expr> - Calculate mathematical expression.";
    } else if (strcmp(cmd, "quicknote") == 0) {
        return "quicknote [add <text>|list|search <term>|delete <id>] - Quick notes.";
    }
    
    snprintf(help_text, sizeof(help_text), "%s - No detailed help available. Try 'help' for commands list.", cmd);
    return help_text;
}
