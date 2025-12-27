/**
 * Suggestion Engine Implementation - Linux Only
 * Provides real-time command suggestions and path completion
 * All using direct system calls for file/directory access
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define PATH_SEP '/'

#include "suggestion_engine.h"

// ============ Command Database ============

static CommandInfo command_database[] = {
    {"ls", "List directory contents", "ls [path]", {"ls", "ls /home", "ls -a"}},
    {"pwd", "Print working directory", "pwd", {"pwd", "", ""}},
    {"cd", "Change directory", "cd <path>", {"cd /home", "cd ..", "cd ~"}},
    {"mkdir", "Create directory", "mkdir <name>", {"mkdir test", "mkdir -p a/b/c", ""}},
    {"rmdir", "Remove empty directory", "rmdir <name>", {"rmdir test", "", ""}},
    {"touch", "Create file or update timestamp", "touch <file>", {"touch file.txt", "", ""}},
    {"rm", "Remove file", "rm <file>", {"rm file.txt", "rm -f old.log", ""}},
    {"cat", "Display file contents", "cat <file>", {"cat file.txt", "cat -n script.sh", ""}},
    {"cp", "Copy file", "cp <src> <dest>", {"cp a.txt b.txt", "cp -r dir1 dir2", ""}},
    {"mv", "Move or rename file", "mv <src> <dest>", {"mv old.txt new.txt", "", ""}},
    {"echo", "Print text", "echo <text>", {"echo hello", "echo $PATH", ""}},
    {"tree", "Directory tree view", "tree [path]", {"tree", "tree /home", ""}},
    {"search", "Search in files", "search <pattern>", {"search hello", "search TODO", ""}},
    {"backup", "Create timestamped backup", "backup <file>", {"backup data.txt", "", ""}},
    {"compare", "Compare two files", "compare <f1> <f2>", {"compare a.txt b.txt", "", ""}},
    {"stats", "Shell statistics", "stats", {"stats", "", ""}},
    {"sysmon", "System resource monitor", "sysmon [-l|-c]", {"sysmon", "sysmon -l", "sysmon -c"}},
    {"bookmark", "Manage bookmarks", "bookmark [name] [path]", {"bookmark", "bookmark work .", "bookmark work"}},
    {"recent", "Recently modified files", "recent", {"recent", "", ""}},
    {"bulk_rename", "Rename multiple files", "bulk_rename <pat> <rep>", {"bulk_rename .txt .md", "", ""}},
    {"help", "Show help", "help [command]", {"help", "help ls", ""}},
    {"history", "Command history", "history", {"history", "", ""}},
    {"clear", "Clear screen", "clear", {"clear", "", ""}},
    {"exit", "Exit shell", "exit", {"exit", "", ""}},
    {"watch", "Watch file for changes", "watch <path> [interval]", {"watch .", "watch log.txt 1000", ""}},
    {"fileinfo", "Detailed file info", "fileinfo <file>", {"fileinfo data.txt", "", ""}},
    {"dirtree", "Directory tree with sizes", "dirtree [path] [-s size]", {"dirtree", "dirtree -s 1M", ""}},
    {"duplicate", "Find duplicate files", "duplicate [path]", {"duplicate", "duplicate /home", ""}},
    {"encrypt", "Encrypt file", "encrypt <file> <key>", {"encrypt secret.txt mykey", "", ""}},
    {"decrypt", "Decrypt file", "decrypt <file> <key>", {"decrypt secret.txt mykey", "", ""}},
    {"hexdump", "Hex view of file", "hexdump <file> [off] [len]", {"hexdump binary.dat", "hexdump file 0 100", ""}},
    {"sizeof", "Total size of matching files", "sizeof <pattern>", {"sizeof *.txt", "sizeof *.c", ""}},
    {"age", "Find files by age", "age <days> [older|newer]", {"age 7 older", "age 1 newer", ""}},
    {"monitor", "Monitor command output", "monitor <sec> <cmd>", {"monitor 5 ls", "", ""}},
    {"freq", "Word frequency analysis", "freq <file> [top_n]", {"freq doc.txt", "freq doc.txt 10", ""}},
    {"lines", "Line/word/char count", "lines <file>", {"lines code.c", "", ""}},
    {"template", "Create from template", "template <tmpl> <out> [vars]", {"template t.txt o.txt name=John", "", ""}},
    {"snap", "Directory snapshots", "snap [save|compare|list] [name]", {"snap save v1", "snap compare v1", "snap list"}},
    {"quicknote", "Quick notes", "quicknote [add|list|search|del]", {"quicknote add Buy milk", "quicknote list", "quicknote search milk"}},
    {"calc", "Calculator", "calc <expression>", {"calc 2+2", "calc 100/4", "calc 2^10"}},
    {"env", "Environment variables", "env [set|get|list]", {"env list", "env get PATH", "env set MY_VAR=test"}},
    {"alias", "Command aliases", "alias [name=cmd|list|remove]", {"alias ll=ls -l", "alias list", "alias remove ll"}},
    {"procinfo", "Process information", "procinfo <pid>", {"procinfo 1234", "", ""}},
    {"netstat", "Network connections", "netstat", {"netstat", "", ""}},
    {"memmap", "Memory map", "memmap [pid]", {"memmap", "memmap 1234", ""}},
    {"jsoncat", "Pretty print JSON", "jsoncat <file>", {"jsoncat data.json", "", ""}},
    {"tail_live", "Live tail with filter", "tail_live <file> [filter]", {"tail_live log.txt", "tail_live log.txt ERROR", ""}},
    {"head", "First N lines", "head <file> [n]", {"head file.txt", "head file.txt 20", ""}},
    {"tail", "Last N lines", "tail <file> [n]", {"tail file.txt", "tail file.txt 20", ""}},
    {"wc", "Word count", "wc <file>", {"wc document.txt", "", ""}},
    {"grep", "Search pattern", "grep <pattern> <file>", {"grep error log.txt", "", ""}},
    {"sort", "Sort lines", "sort <file> [-r] [-n]", {"sort list.txt", "sort nums.txt -n", ""}},
    {"uniq", "Remove duplicates", "uniq <file>", {"uniq list.txt", "", ""}},
    {"rev", "Reverse lines", "rev <file>", {"rev file.txt", "", ""}},
    {"date", "Current date/time", "date [format]", {"date", "", ""}},
    {"whoami", "Current user", "whoami", {"whoami", "", ""}},
    {"hostname", "System hostname", "hostname", {"hostname", "", ""}},
    {"uptime", "System uptime", "uptime", {"uptime", "", ""}},
    {"df", "Disk free space", "df", {"df", "", ""}},
    {"ps", "Process list", "ps", {"ps", "", ""}},
    {"kill", "Kill process", "kill <pid> [signal]", {"kill 1234", "kill 1234 9", ""}},
    {"undo", "Undo last command", "undo", {"undo", "", ""}},
    {"macro", "Macro management", "macro [define|run|list] <name>", {"macro define m1", "macro run m1", "macro list"}},
};

static int num_command_infos = sizeof(command_database) / sizeof(command_database[0]);

// History storage
static char command_history_storage[MAX_HISTORY_SUGGESTIONS][MAX_SUGGESTION_LEN];
static int history_count = 0;

// ============ Helper Functions ============

static void str_to_lower_sug(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

// Levenshtein distance for fuzzy matching
static int levenshtein_distance(const char *s1, const char *s2) {
    int len1 = strlen(s1);
    int len2 = strlen(s2);
    
    if (len1 == 0) return len2;
    if (len2 == 0) return len1;
    
    // Use simple array for small strings
    int matrix[64][64];
    if (len1 >= 64 || len2 >= 64) return 100;  // Too long
    
    for (int i = 0; i <= len1; i++) matrix[i][0] = i;
    for (int j = 0; j <= len2; j++) matrix[0][j] = j;
    
    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int cost = (tolower(s1[i-1]) == tolower(s2[j-1])) ? 0 : 1;
            int del = matrix[i-1][j] + 1;
            int ins = matrix[i][j-1] + 1;
            int sub = matrix[i-1][j-1] + cost;
            
            matrix[i][j] = del < ins ? (del < sub ? del : sub) : (ins < sub ? ins : sub);
        }
    }
    
    return matrix[len1][len2];
}

// ============ Main Functions ============

void suggestion_init(void) {
    history_count = 0;
}

void suggestion_get_commands(const char *prefix, SuggestionList *out) {
    if (!out) return;
    out->count = 0;
    out->selected_index = 0;
    
    if (!prefix || strlen(prefix) == 0) {
        return;
    }
    
    char lower_prefix[256];
    strncpy(lower_prefix, prefix, sizeof(lower_prefix) - 1);
    lower_prefix[sizeof(lower_prefix) - 1] = '\0';
    str_to_lower_sug(lower_prefix);
    int prefix_len = strlen(lower_prefix);
    
    // Exact prefix matches first
    for (int i = 0; i < num_command_infos && out->count < MAX_SUGGESTIONS; i++) {
        char lower_cmd[64];
        strncpy(lower_cmd, command_database[i].name, sizeof(lower_cmd) - 1);
        str_to_lower_sug(lower_cmd);
        
        if (strncmp(lower_cmd, lower_prefix, prefix_len) == 0) {
            strncpy(out->suggestions[out->count], command_database[i].name, MAX_SUGGESTION_LEN - 1);
            out->count++;
        }
    }
    
    // Fuzzy matches if not enough
    if (out->count < 3 && prefix_len >= 2) {
        for (int i = 0; i < num_command_infos && out->count < MAX_SUGGESTIONS; i++) {
            int dist = levenshtein_distance(lower_prefix, command_database[i].name);
            if (dist <= 2 && dist > 0) {
                // Check if not already added
                int found = 0;
                for (int j = 0; j < out->count; j++) {
                    if (strcmp(out->suggestions[j], command_database[i].name) == 0) {
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    strncpy(out->suggestions[out->count], command_database[i].name, MAX_SUGGESTION_LEN - 1);
                    out->count++;
                }
            }
        }
    }
}

void suggestion_get_paths(const char *partial_path, int dirs_only, SuggestionList *out) {
    if (!out) return;
    out->count = 0;
    out->selected_index = 0;
    
    char dir_path[512] = ".";
    char file_prefix[256] = "";
    
    if (partial_path && strlen(partial_path) > 0) {
        // Find the last separator
        const char *last_sep = strrchr(partial_path, '/');
        
        if (last_sep) {
            // Has directory component
            int dir_len = last_sep - partial_path;
            if (dir_len > 0) {
                strncpy(dir_path, partial_path, dir_len);
                dir_path[dir_len] = '\0';
            } else {
                strcpy(dir_path, "/");
            }
            strncpy(file_prefix, last_sep + 1, sizeof(file_prefix) - 1);
        } else {
            strncpy(file_prefix, partial_path, sizeof(file_prefix) - 1);
        }
    }
    
    // Use opendir/readdir system calls
    DIR *dir = opendir(dir_path);
    if (!dir) return;
    
    struct dirent *entry;
    int prefix_len = strlen(file_prefix);
    
    while ((entry = readdir(dir)) != NULL && out->count < MAX_SUGGESTIONS) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // Check prefix match
        if (prefix_len > 0 && strncmp(entry->d_name, file_prefix, prefix_len) != 0) {
            continue;
        }
        
        // Check if directory
        char full_path[768];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        struct stat st;
        if (stat(full_path, &st) == 0) {
            int is_dir = S_ISDIR(st.st_mode);
            
            if (dirs_only && !is_dir) continue;
            
            // Build suggestion - use larger buffer to avoid truncation
            char suggestion[768];
            if (strcmp(dir_path, ".") == 0) {
                snprintf(suggestion, sizeof(suggestion), "%s%s", 
                        entry->d_name, is_dir ? "/" : "");
            } else {
                snprintf(suggestion, sizeof(suggestion), "%s/%s%s",
                        dir_path, entry->d_name, is_dir ? "/" : "");
            }
            
            strncpy(out->suggestions[out->count], suggestion, MAX_SUGGESTION_LEN - 1);
            out->suggestions[out->count][MAX_SUGGESTION_LEN - 1] = '\0';
            out->count++;
        }
    }
    
    closedir(dir);
}

void suggestion_get_contextual(const char *cmd, const char *partial_arg, SuggestionList *out) {
    if (!out) return;
    out->count = 0;
    
    // Commands that need directory suggestions
    const char *dir_cmds[] = {"cd", "mkdir", "rmdir", "tree", "dirtree", "watch"};
    for (int i = 0; i < 6; i++) {
        if (strcmp(cmd, dir_cmds[i]) == 0) {
            suggestion_get_paths(partial_arg, 1, out);  // dirs only
            return;
        }
    }
    
    // Commands that need file suggestions
    const char *file_cmds[] = {"cat", "rm", "touch", "head", "tail", "wc", "grep", 
                               "hexdump", "fileinfo", "backup", "encrypt", "decrypt",
                               "jsoncat", "freq", "lines", "sort", "uniq", "rev"};
    for (int i = 0; i < 18; i++) {
        if (strcmp(cmd, file_cmds[i]) == 0) {
            suggestion_get_paths(partial_arg, 0, out);  // all files
            return;
        }
    }
    
    // cp, mv - both source and dest need paths
    if (strcmp(cmd, "cp") == 0 || strcmp(cmd, "mv") == 0 || strcmp(cmd, "compare") == 0) {
        suggestion_get_paths(partial_arg, 0, out);
        return;
    }
}

void suggestion_add_to_history(const char *cmd) {
    if (!cmd || strlen(cmd) == 0) return;
    
    // Check if already in history
    for (int i = 0; i < history_count; i++) {
        if (strcmp(command_history_storage[i], cmd) == 0) {
            return;  // Already exists
        }
    }
    
    // Add to history
    if (history_count < MAX_HISTORY_SUGGESTIONS) {
        strncpy(command_history_storage[history_count], cmd, MAX_SUGGESTION_LEN - 1);
        command_history_storage[history_count][MAX_SUGGESTION_LEN - 1] = '\0';
        history_count++;
    } else {
        // Shift and add
        for (int i = 0; i < history_count - 1; i++) {
            strcpy(command_history_storage[i], command_history_storage[i + 1]);
        }
        strncpy(command_history_storage[history_count - 1], cmd, MAX_SUGGESTION_LEN - 1);
    }
}

void suggestion_get_from_history(const char *prefix, SuggestionList *out) {
    if (!out) return;
    out->count = 0;
    out->selected_index = 0;
    
    int prefix_len = prefix ? strlen(prefix) : 0;
    
    // Search from most recent
    for (int i = history_count - 1; i >= 0 && out->count < MAX_SUGGESTIONS; i--) {
        if (prefix_len == 0 || strncmp(command_history_storage[i], prefix, prefix_len) == 0) {
            strncpy(out->suggestions[out->count], command_history_storage[i], MAX_SUGGESTION_LEN - 1);
            out->count++;
        }
    }
}

CommandInfo* suggestion_get_command_info(const char *cmd) {
    if (!cmd) return NULL;
    
    for (int i = 0; i < num_command_infos; i++) {
        if (strcmp(command_database[i].name, cmd) == 0) {
            return &command_database[i];
        }
    }
    return NULL;
}

int suggestion_fuzzy_score(const char *str, const char *pattern) {
    if (!str || !pattern) return 0;
    
    int str_len = strlen(str);
    int pat_len = strlen(pattern);
    
    if (pat_len == 0) return 100;
    if (str_len == 0) return 0;
    
    // Exact prefix match - highest score
    if (strncasecmp(str, pattern, pat_len) == 0) {
        return 100 - (str_len - pat_len);  // Shorter is better
    }
    
    // Substring match
    char lower_str[256], lower_pat[256];
    strncpy(lower_str, str, sizeof(lower_str) - 1);
    strncpy(lower_pat, pattern, sizeof(lower_pat) - 1);
    str_to_lower_sug(lower_str);
    str_to_lower_sug(lower_pat);
    
    if (strstr(lower_str, lower_pat)) {
        return 70;
    }
    
    // Levenshtein-based score
    int dist = levenshtein_distance(lower_str, lower_pat);
    if (dist == 0) return 100;
    if (dist == 1) return 80;
    if (dist == 2) return 60;
    if (dist <= 4) return 40;
    
    return 0;
}
