/**
 * NLP Engine Implementation - Natural Language Processing for Shell Commands
 * Pure C implementation using pattern matching and keyword extraction
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "nlp_engine.h"

// ============ Pattern Definitions ============

typedef struct {
    const char *patterns[10];  // Multiple patterns that match
    int pattern_count;
    const char *command_template;
    const char *explanation;
} NLPPattern;

// All supported NLP patterns
static NLPPattern nlp_patterns[] = {
    // Show/List files
    {{"show files", "list files", "display files", "show all files", "list all files", 
      "what files", "see files", "view files"}, 8, 
     "ls", "Listing files in current directory"},
    
    // Show directory tree
    {{"show tree", "display tree", "directory tree", "show directory tree", "folder structure"}, 5,
     "tree", "Displaying directory tree structure"},
    
    // Current directory
    {{"where am i", "current directory", "current path", "current location", 
      "show directory", "print directory", "what directory", "pwd"}, 8,
     "pwd", "Showing current working directory"},
    
    // Create directory
    {{"create folder", "make folder", "new folder", "create directory", 
      "make directory", "new directory", "mkdir"}, 7,
     "mkdir %s", "Creating new directory"},
    
    // Create file
    {{"create file", "make file", "new file", "touch file", "create new file"}, 5,
     "touch %s", "Creating new file"},
    
    // Delete file
    {{"delete file", "remove file", "erase file", "delete the file", "rm file"}, 5,
     "rm %s", "Removing file"},
    
    // Delete directory
    {{"delete folder", "remove folder", "delete directory", "remove directory", 
      "erase folder", "rmdir"}, 6,
     "rmdir %s", "Removing directory"},
    
    // Copy file
    {{"copy file", "duplicate file", "copy the file", "make copy of"}, 4,
     "cp %s %s", "Copying file"},
    
    // Move/Rename file
    {{"move file", "rename file", "move the file", "rename the file", "relocate"}, 5,
     "mv %s %s", "Moving/renaming file"},
    
    // Read file
    {{"read file", "show file", "display file", "print file", "view file", 
      "cat file", "show contents", "what is in", "whats in"}, 9,
     "cat %s", "Displaying file contents"},
    
    // Search
    {{"search for", "find text", "look for", "search text", "grep for"}, 5,
     "search %s", "Searching for pattern"},
    
    // Change directory
    {{"go to", "change to", "navigate to", "switch to", "cd to", "enter folder", "enter directory"}, 7,
     "cd %s", "Changing directory"},
    
    // Go back
    {{"go back", "go up", "parent directory", "go to parent", "cd .."}, 5,
     "cd ..", "Going to parent directory"},
    
    // Go home
    {{"go home", "home directory", "cd home", "go to home"}, 4,
     "cd ~", "Going to home directory"},
    
    // System monitor
    {{"system monitor", "show system", "system info", "system status", 
      "resource monitor", "show resources", "cpu usage", "memory usage"}, 8,
     "sysmon", "Opening system resource monitor"},
    
    // Help
    {{"help", "show help", "help me", "what commands", "available commands", "show commands"}, 6,
     "help", "Showing available commands"},
    
    // History
    {{"show history", "command history", "previous commands", "history"}, 4,
     "history", "Showing command history"},
    
    // Clear screen
    {{"clear screen", "clear terminal", "cls", "clear"}, 4,
     "clear", "Clearing the screen"},
    
    // Recent files
    {{"recent files", "show recent", "recently modified", "new files"}, 4,
     "recent", "Showing recently modified files"},
    
    // Backup
    {{"backup file", "create backup", "save backup", "backup"}, 4,
     "backup %s", "Creating file backup"},
    
    // Compare files
    {{"compare files", "diff files", "check difference", "compare"}, 4,
     "compare %s %s", "Comparing two files"},
    
    // File info
    {{"file info", "file details", "file information", "info about"}, 4,
     "fileinfo %s", "Showing detailed file information"},
    
    // Find duplicates
    {{"find duplicates", "duplicate files", "find duplicate"}, 3,
     "duplicate", "Finding duplicate files"},
    
    // Word count
    {{"count words", "word count", "count lines", "line count", "wc"}, 5,
     "wc %s", "Counting words/lines in file"},
    
    // Head/Tail
    {{"first lines", "show first", "head of file", "beginning of"}, 4,
     "head %s", "Showing first lines of file"},
    
    {{"last lines", "show last", "tail of file", "end of"}, 4,
     "tail %s", "Showing last lines of file"},
    
    // Date/Time
    {{"current time", "what time", "show time", "current date", "show date", "date and time"}, 6,
     "date", "Showing current date and time"},
    
    // User info
    {{"who am i", "current user", "my username", "whoami"}, 4,
     "whoami", "Showing current user"},
    
    // Disk space
    {{"disk space", "free space", "storage space", "disk usage", "df"}, 5,
     "df", "Showing disk space usage"},
    
    // Process list
    {{"running processes", "list processes", "show processes", "process list", "ps"}, 5,
     "ps", "Listing running processes"},
    
    // Calculator
    {{"calculate", "calc", "compute", "math"}, 4,
     "calc %s", "Calculating expression"},
    
    // Notes
    {{"add note", "quick note", "save note", "take note"}, 4,
     "quicknote add %s", "Adding a quick note"},
    
    {{"show notes", "list notes", "my notes"}, 3,
     "quicknote list", "Showing saved notes"},
    
    // Exit
    {{"exit", "quit", "close", "bye", "goodbye"}, 5,
     "exit", "Exiting the shell"},
    
    // Compress
    {{"compress file", "compress", "zip file", "gzip file", "compress this"}, 5,
     "compress %s", "Compressing file"},
    
    {{"decompress file", "decompress", "unzip file", "extract file", "uncompress"}, 5,
     "compress %s d", "Decompressing file"},
    
    // Convert
    {{"convert file", "convert", "change format", "convert to", "transform file"}, 5,
     "convert %s %s", "Converting file format"},
};

static int num_patterns = sizeof(nlp_patterns) / sizeof(nlp_patterns[0]);

// ============ Available Commands List ============

static const char *available_commands[] = {
    "ls", "pwd", "cd", "mkdir", "rmdir", "touch", "rm", "cat", "cp", "mv",
    "echo", "tree", "search", "backup", "compare", "stats", "sysmon",
    "bookmark", "recent", "bulk_rename", "help", "history", "exit", "clear",
    "watch", "fileinfo", "dirtree", "duplicate", "encrypt", "decrypt",
    "hexdump", "sizeof", "age", "monitor", "freq", "lines", "template",
    "snap", "quicknote", "calc", "env", "alias", "procinfo", "netstat",
    "memmap", "jsoncat", "tail_live", "head", "tail", "wc", "grep",
    "sort", "uniq", "rev", "date", "whoami", "hostname", "uptime", "df",
    "ps", "kill", "undo", "macro", "teach", "compress", "convert", "q"
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
    
    // Check each pattern
    for (int i = 0; i < num_patterns; i++) {
        NLPPattern *p = &nlp_patterns[i];
        
        for (int j = 0; j < p->pattern_count; j++) {
            if (str_contains(trimmed, p->patterns[j])) {
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
                    
                    // Single argument
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
        "can you", "i want", "i need"
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
    } else if (strcmp(cmd, "compress") == 0) {
        return "compress <file> [d] - Compress file with gzip or decompress with 'd' flag.";
    } else if (strcmp(cmd, "convert") == 0) {
        return "convert <input> <output> - Convert file formats (txt, md, html, csv).";
    }
    
    snprintf(help_text, sizeof(help_text), "%s - No detailed help available. Try 'help' for commands list.", cmd);
    return help_text;
}
