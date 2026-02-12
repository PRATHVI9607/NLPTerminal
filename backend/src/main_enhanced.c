/**
 * NLP Terminal - Enhanced Main Shell with NLP Support
 * All command execution in C using system calls
 * Supports real-time suggestions and natural language input
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <signal.h>

#define PATH_SEP '/'

#include "history.h"
#include "utils.h"
#include "trie.h"
#include "bktree.h"
#include "undo.h"
#include "macros.h"
#include "commands.h"
#include "nlp_engine.h"
#include "suggestion_engine.h"
#include "custom_commands.h"
#include "sysmon_advanced.h"
#include "visualizer.h"
#include "expression.h"
#include "utilities.h"
#include "os_commands.h"
#include "ipc_commands.h"

#define MAX_CMD_LEN 1024
#define MAX_ARGS 64

// Global state
int teaching_mode = 0;
int recording_macro = 0;
int suggestion_mode = 1;  // Enable real-time suggestions by default

// Forward declarations
void execute_line(char *cmd, History *history, TrieNode *trie, BKTreeNode *bktree, UndoStack *undo_stack);
void show_suggestions(const char *partial);
void process_nlp_command(const char *input, char *output);

// ============ PROMPT AND INPUT ============

void type_prompt() {
    char cwd[1024];
    if (recording_macro) {
        printf("macro_rec> ");
    } else if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s> ", cwd);
    } else {
        printf("shell> ");
    }
    fflush(stdout);
}

void read_command(char *cmd) {
    if (fgets(cmd, MAX_CMD_LEN, stdin) == NULL) {
        printf("\n");
        exit(0);
    }
    cmd[strcspn(cmd, "\n")] = 0;
}

// Smart parse_command that handles quoted strings properly
void parse_command(char *cmd, char **args) {
    int i = 0;
    int in_quotes = 0;
    char *start = cmd;
    char *p = cmd;
    
    // Skip leading whitespace
    while (*p && isspace((unsigned char)*p)) p++;
    start = p;
    
    while (*p && i < MAX_ARGS - 1) {
        if (*p == '"' || *p == '\'') {
            char quote = *p;
            // Start or end of quoted string
            if (!in_quotes) {
                in_quotes = 1;
                start = p + 1;  // Start after the quote
                p++;
                continue;
            } else if (*p == quote) {
                // End of quoted string
                *p = '\0';
                args[i++] = start;
                in_quotes = 0;
                p++;
                // Skip whitespace after quote
                while (*p && isspace((unsigned char)*p)) p++;
                start = p;
                continue;
            }
        } else if (isspace((unsigned char)*p) && !in_quotes) {
            // End of unquoted token
            if (p > start) {
                *p = '\0';
                args[i++] = start;
            }
            p++;
            // Skip consecutive whitespace
            while (*p && isspace((unsigned char)*p)) p++;
            start = p;
            continue;
        }
        p++;
    }
    
    // Handle last token
    if (*start && i < MAX_ARGS - 1) {
        // Remove trailing quote if present
        int len = strlen(start);
        if (len > 0 && (start[len-1] == '"' || start[len-1] == '\'')) {
            start[len-1] = '\0';
        }
        args[i++] = start;
    }
    
    args[i] = NULL;
}

// ============ SUGGESTION HANDLING ============

void show_suggestions(const char *partial) {
    if (!partial || strlen(partial) == 0) return;
    
    SuggestionList suggestions;
    suggestion_get_commands(partial, &suggestions);
    
    if (suggestions.count > 0) {
        printf("\033[90m");  // Gray color
        printf(" → ");
        for (int i = 0; i < suggestions.count && i < 5; i++) {
            printf("%s", suggestions.suggestions[i]);
            if (i < suggestions.count - 1 && i < 4) printf(" | ");
        }
        printf("\033[0m");
    }
}

// Handle SUGGEST command from frontend
void handle_suggest_command(const char *partial) {
    SuggestionList cmd_suggestions;
    suggestion_get_commands(partial, &cmd_suggestions);
    
    printf("SUGGESTIONS:");
    for (int i = 0; i < cmd_suggestions.count; i++) {
        printf("%s", cmd_suggestions.suggestions[i]);
        if (i < cmd_suggestions.count - 1) printf("|");
    }
    printf("\n");
    fflush(stdout);
}

// Handle contextual suggestions (for arguments)
void handle_context_suggest(const char *cmd, const char *partial) {
    SuggestionList suggestions;
    suggestion_get_contextual(cmd, partial, &suggestions);
    
    printf("SUGGESTIONS:");
    for (int i = 0; i < suggestions.count; i++) {
        printf("%s", suggestions.suggestions[i]);
        if (i < suggestions.count - 1) printf("|");
    }
    printf("\n");
    fflush(stdout);
}

// ============ NLP PROCESSING ============

void process_nlp_command(const char *input, char *output) {
    NLPResult result = nlp_translate(input);
    
    if (result.was_translated) {
        strcpy(output, result.translated);
        printf("NLP_TRANSLATED:%s:%s\n", result.translated, result.explanation);
    } else {
        strcpy(output, result.original);
    }
    fflush(stdout);
}

// ============ HELP SYSTEM ============

void show_help(char **args) {
    if (args[1] != NULL) {
        // Show help for specific command
        const char *help = nlp_get_command_help(args[1]);
        printf("%s\n", help);
        return;
    }
    
    printf("\n");
    printf("========================================\n");
    printf("  NLP TERMINAL - COMMAND REFERENCE\n");
    printf("========================================\n\n");
    
    printf("FILE OPERATIONS:\n");
    printf("  ls [path]          - List directory contents\n");
    printf("  pwd                - Print current directory\n");
    printf("  cd <path>          - Change directory\n");
    printf("  mkdir <name>       - Create directory\n");
    printf("  rmdir <name>       - Remove empty directory\n");
    printf("  touch <file>       - Create file\n");
    printf("  rm <file>          - Remove file\n");
    printf("  cat <file>         - Display file contents\n");
    printf("  cp <src> <dst>     - Copy file\n");
    printf("  mv <src> <dst>     - Move/rename file\n");
    printf("  echo <text>        - Print text\n\n");
    
    printf("ADVANCED FILE OPERATIONS:\n");
    printf("  tree [path]        - Directory tree view\n");
    printf("  search <pattern>   - Search in files\n");
    printf("  backup <file>      - Create timestamped backup\n");
    printf("  compare <f1> <f2>  - Compare two files\n");
    printf("  head <file> [n]    - Show first n lines\n");
    printf("  tail <file> [n]    - Show last n lines\n");
    printf("  wc <file>          - Word/line/char count\n");
    printf("  grep <pat> <file>  - Search pattern in file\n");
    printf("  sort <file> [-r]   - Sort lines\n");
    printf("  uniq <file>        - Remove duplicate lines\n");
    printf("  rev <file>         - Reverse lines\n\n");
    
    printf("UNIQUE COMMANDS (Not in standard UNIX):\n");
    printf("  fileinfo <file>    - Detailed file info (size, hash, permissions)\n");
    printf("  hexdump <file>     - Hex view of file contents\n");
    printf("  duplicate [path]   - Find duplicate files by content\n");
    printf("  encrypt <f> <key>  - Encrypt file with XOR cipher\n");
    printf("  decrypt <f> <key>  - Decrypt file\n");
    printf("  sizeof <pattern>   - Total size of matching files\n");
    printf("  age <days> [o|n]   - Find files older/newer than days\n");
    printf("  freq <file> [n]    - Word frequency analysis\n");
    printf("  lines <file>       - Detailed line/word/char statistics\n");
    printf("  quicknote          - Quick note taking (add/list/search/clear)\n");
    printf("  calc <expr>        - Calculator (supports +,-,*,/,^)\n\n");
    
    printf("SYSTEM INFORMATION:\n");
    printf("  sysmon             - Full system resource monitor\n");
    printf("  sysmon -c          - Compact system info\n");
    printf("  sysmon -l          - Live updating monitor\n");
    printf("  ps                 - List running processes\n");
    printf("  kill <pid> [sig]   - Kill process\n");
    printf("  df                 - Disk free space\n");
    printf("  uptime             - System uptime\n");
    printf("  date               - Current date/time\n");
    printf("  whoami             - Current user\n");
    printf("  hostname           - System hostname\n\n");
    
    printf("SHELL FEATURES:\n");
    printf("  history            - Show command history\n");
    printf("  bookmark [n] [p]   - Manage directory bookmarks\n");
    printf("  recent             - Recently modified files\n");
    printf("  bulk_rename <p><r> - Rename multiple files\n");
    printf("  stats              - Shell statistics\n");
    printf("  undo               - Undo last command\n");
    printf("  macro              - Macro recording (define/run/list)\n");
    printf("  teach [on|off]     - Teaching mode\n");
    printf("  clear              - Clear screen\n");
    printf("  exit               - Exit shell\n\n");
    
    printf("NATURAL LANGUAGE EXAMPLES:\n");
    printf("  \"show all files\"            -> ls\n");
    printf("  \"create folder called test\" -> mkdir test\n");
    printf("  \"what's in myfile.txt\"      -> cat myfile.txt\n");
    printf("  \"go to home\"                -> cd ~\n");
    printf("  \"system monitor\"            -> sysmon\n");
    printf("  \"find duplicates\"           -> duplicate\n");
    printf("  \"calculate 2+2*3\"           -> calc 2+2*3\n\n");
}

// ============ COMMAND EXPLANATION (Teaching Mode) ============

void explain_command(const char *cmd) {
    printf("\n\033[36m[Teaching Mode]\033[0m ");
    
    const char *help = nlp_get_command_help(cmd);
    printf("%s\n", help);
}

// ============ MAIN EXECUTION ============

void execute_line(char *cmd, History *history, TrieNode *trie, BKTreeNode *bktree, UndoStack *undo_stack) {
    char *args[MAX_ARGS];
    
    // Handle special frontend commands
    if (strncmp(cmd, "SUGGEST:", 8) == 0) {
        handle_suggest_command(cmd + 8);
        return;
    }
    
    if (strncmp(cmd, "CONTEXT:", 8) == 0) {
        char *space = strchr(cmd + 8, ' ');
        if (space) {
            *space = '\0';
            handle_context_suggest(cmd + 8, space + 1);
        }
        return;
    }
    
    // Auto-detect and process natural language input
    if (strncmp(cmd, "NLP:", 4) == 0) {
        char translated[MAX_CMD_LEN];
        process_nlp_command(cmd + 4, translated);
        strcpy(cmd, translated);
    } else if (nlp_is_natural_language(cmd)) {
        // Automatically translate natural language
        char translated[MAX_CMD_LEN];
        process_nlp_command(cmd, translated);
        strcpy(cmd, translated);
    }
    
    // Empty command
    if (strlen(cmd) == 0) {
        return;
    }
    
    // Exit command
    if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
        printf("Goodbye!\n");
        exit(0);
    }
    
    // History command - supports: history, history <n>, !<n>
    if (strcmp(cmd, "history") == 0) {
        print_history(history);
        return;
    }
    if (strncmp(cmd, "history ", 8) == 0) {
        int index = atoi(cmd + 8);
        if (index > 0 && index <= history->count) {
            const char *hist_cmd = get_history_at(history, index - 1);
            if (hist_cmd) {
                printf("Running: %s\n", hist_cmd);
                char cmd_copy[MAX_CMD_LEN];
                strcpy(cmd_copy, hist_cmd);
                execute_line(cmd_copy, history, trie, bktree, undo_stack);
            }
        } else {
            printf("history: %d: event not found (valid range: 1-%d)\n", index, history->count);
        }
        return;
    }
    // Support !n syntax to run command from history
    if (cmd[0] == '!' && cmd[1] >= '0' && cmd[1] <= '9') {
        int index = atoi(cmd + 1);
        if (index > 0 && index <= history->count) {
            const char *hist_cmd = get_history_at(history, index - 1);
            if (hist_cmd) {
                printf("Running: %s\n", hist_cmd);
                char cmd_copy[MAX_CMD_LEN];
                strcpy(cmd_copy, hist_cmd);
                execute_line(cmd_copy, history, trie, bktree, undo_stack);
            }
        } else {
            printf("!%d: event not found (valid range: 1-%d)\n", index, history->count);
        }
        return;
    }
    
    // Help command
    if (strcmp(cmd, "help") == 0 || strncmp(cmd, "help ", 5) == 0) {
        char cmd_copy[MAX_CMD_LEN];
        strcpy(cmd_copy, cmd);
        parse_command(cmd_copy, args);
        show_help(args);
        return;
    }
    
    // Categories command - show all command categories
    if (strcmp(cmd, "categories") == 0) {
        printf("\n");
        printf("========================================\n");
        printf("  NLP TERMINAL - ALL COMMANDS (86)\n");
        printf("========================================\n\n");
        
        printf("FILE OPERATIONS (11):\n");
        printf("  ls, cd, pwd, mkdir, rmdir, touch, rm, cat, cp, mv, echo\n\n");
        
        printf("ADVANCED FILE OPS (13):\n");
        printf("  tree, search, fileinfo, hexdump, duplicate, compare, backup,\n");
        printf("  encrypt, decrypt, sizeof, age, freq, lines\n\n");
        
        printf("TEXT PROCESSING (8):\n");
        printf("  head, tail, wc, grep, sort, uniq, rev, split\n\n");
        
        printf("SYSTEM INFORMATION (8):\n");
        printf("  sysmon, date, whoami, hostname, uptime, df, ps, uname\n\n");
        
        printf("PROCESS MANAGEMENT (2):\n");
        printf("  kill, progress\n\n");
        
        printf("USER MANAGEMENT (3):\n");
        printf("  adduser, deluser, chpasswd\n\n");
        
        printf("PERMISSIONS (2):\n");
        printf("  chmod, chown\n\n");
        
        printf("COMPRESSION (3):\n");
        printf("  compress, decompress, archive\n\n");
        
        printf("MATHEMATICAL (8):\n");
        printf("  calc, infix2postfix, infix2prefix, postfix2infix,\n");
        printf("  prefix2infix, evaluate, exprtype, visualeval\n\n");
        
        printf("VISUALIZATION (3):\n");
        printf("  visualize, screen, tree\n\n");
        
        printf("CUSTOM UTILITIES (13):\n");
        printf("  history, undo, macro, bookmark, recent, bulk_rename,\n");
        printf("  stats, teach, quicknote, json, calendar, ping, wget\n\n");
        
        printf("SHELL UTILITIES (3):\n");
        printf("  help, exit, clear\n\n");
        
        printf("NATURAL LANGUAGE (Auto-detected):\n");
        printf("  Just speak naturally! Examples:\n");
        printf("  - \"show me all files\" -> ls -la\n");
        printf("  - \"create folder called test\" -> mkdir test\n");
        printf("  - \"where am i\" -> pwd\n");
        printf("  - \"current time\" -> date\n");
        printf("  - \"disk space\" -> df -h\n\n");
        
        printf("Type 'help COMMAND' for detailed information on any command.\n\n");
        return;
    }
    
    // Teaching mode
    if (strncmp(cmd, "teach ", 6) == 0) {
        if (strcmp(cmd + 6, "on") == 0) {
            teaching_mode = 1;
            printf("Teaching mode enabled. Commands will be explained.\n");
        } else if (strcmp(cmd + 6, "off") == 0) {
            teaching_mode = 0;
            printf("Teaching mode disabled.\n");
        }
        return;
    }
    
    // Undo command
    if (strcmp(cmd, "undo") == 0) {
        execute_undo(undo_stack);
        return;
    }
    
    // Macro handling - handle "macro" alone or "macro <action>"
    if (strcmp(cmd, "macro") == 0) {
        printf("Macro Commands:\n");
        printf("  macro define <name>  - Start recording a macro\n");
        printf("  macro end            - Stop recording\n");
        printf("  macro run <name>     - Run a saved macro\n");
        printf("  macro list           - List all macros\n");
        return;
    }
    
    if (strncmp(cmd, "macro ", 6) == 0) {
        char *action = cmd + 6;
        if (strncmp(action, "define ", 7) == 0) {
            start_recording_macro(action + 7);
            recording_macro = 1;
            printf("Recording macro '%s'. Type 'macro end' to finish.\n", action + 7);
        } else if (strcmp(action, "end") == 0) {
            if (recording_macro) {
                end_recording_macro();
                recording_macro = 0;
                printf("Macro recording ended.\n");
            } else {
                printf("No macro is being recorded.\n");
            }
        } else if (strncmp(action, "run ", 4) == 0) {
            Macro *m = find_macro(action + 4);
            if (m) {
                printf("Running macro '%s'...\n", action + 4);
                MacroStep *step = m->head;
                while (step) {
                    printf(">> %s\n", step->command);
                    char step_cmd[MAX_CMD_LEN];
                    strcpy(step_cmd, step->command);
                    execute_line(step_cmd, history, trie, bktree, undo_stack);
                    step = step->next;
                }
            } else {
                printf("Macro '%s' not found.\n", action + 4);
            }
        } else if (strcmp(action, "list") == 0) {
            list_macros();
        } else {
            printf("Unknown macro action: %s\n", action);
            printf("Use 'macro' for help.\n");
        }
        return;
    }
    
    // If recording macro, add step
    if (recording_macro) {
        if (strcmp(cmd, "macro end") == 0) {
            end_recording_macro();
            recording_macro = 0;
            return;
        }
        add_macro_step(cmd);
        return;
    }
    
    // Suggestion commands
    if (strncmp(cmd, "complete ", 9) == 0) {
        SuggestionList suggestions;
        suggestion_get_commands(cmd + 9, &suggestions);
        printf("Suggestions: ");
        for (int i = 0; i < suggestions.count; i++) {
            printf("%s ", suggestions.suggestions[i]);
        }
        printf("\n");
        return;
    }
    
    // Parse command
    char cmd_copy[MAX_CMD_LEN];
    strcpy(cmd_copy, cmd);
    parse_command(cmd_copy, args);
    
    if (args[0] == NULL) return;
    
    // Add to suggestion history
    suggestion_add_to_history(cmd);
    
    // ============ BUILT-IN COMMANDS ============
    
    // Basic file operations (from commands.c)
    if (strcmp(args[0], "ls") == 0) { 
        do_ls(args); 
        if (teaching_mode) explain_command("ls");
        return; 
    }
    if (strcmp(args[0], "pwd") == 0) { 
        do_pwd(args); 
        if (teaching_mode) explain_command("pwd");
        return; 
    }
    if (strcmp(args[0], "cat") == 0) { 
        do_cat(args); 
        if (teaching_mode) explain_command("cat");
        return; 
    }
    if (strcmp(args[0], "echo") == 0) { 
        do_echo(args); 
        return; 
    }
    if (strcmp(args[0], "tree") == 0) { 
        do_tree(args); 
        if (teaching_mode) explain_command("tree");
        return; 
    }
    if (strcmp(args[0], "search") == 0) { 
        do_search(args); 
        return; 
    }
    if (strcmp(args[0], "backup") == 0) { 
        do_backup(args); 
        return; 
    }
    if (strcmp(args[0], "compare") == 0) { 
        do_compare(args); 
        return; 
    }
    if (strcmp(args[0], "stats") == 0) { 
        do_stats(args); 
        return; 
    }
    if (strcmp(args[0], "bookmark") == 0) { 
        do_bookmark(args); 
        return; 
    }
    if (strcmp(args[0], "recent") == 0) { 
        do_recent(args); 
        return; 
    }
    if (strcmp(args[0], "bulk_rename") == 0) { 
        do_bulk_rename(args); 
        return; 
    }
    
    // Commands with undo support
    if (strcmp(args[0], "mkdir") == 0) { 
        do_mkdir(args); 
        if (args[1]) push_undo(undo_stack, cmd, UNDO_MKDIR, args[1], NULL);
        if (teaching_mode) explain_command("mkdir");
        return; 
    }
    if (strcmp(args[0], "rmdir") == 0) { 
        do_rmdir(args); 
        if (args[1]) push_undo(undo_stack, cmd, UNDO_RMDIR, args[1], NULL);
        if (teaching_mode) explain_command("rmdir");
        return; 
    }
    if (strcmp(args[0], "rm") == 0) { 
        do_rm(args); 
        if (args[1]) push_undo(undo_stack, cmd, UNDO_RM, args[1], NULL);
        if (teaching_mode) explain_command("rm");
        return; 
    }
    if (strcmp(args[0], "touch") == 0) { 
        do_touch(args); 
        if (args[1]) push_undo(undo_stack, cmd, UNDO_TOUCH, args[1], NULL);
        if (teaching_mode) explain_command("touch");
        return; 
    }
    if (strcmp(args[0], "cp") == 0) { 
        do_cp(args); 
        if (args[2]) push_undo(undo_stack, cmd, UNDO_CP, args[2], NULL);
        if (teaching_mode) explain_command("cp");
        return; 
    }
    if (strcmp(args[0], "mv") == 0) { 
        do_mv(args); 
        if (args[1] && args[2]) push_undo(undo_stack, cmd, UNDO_MV, args[2], args[1]);
        if (teaching_mode) explain_command("mv");
        return; 
    }
    
    // ============ CUSTOM COMMANDS ============
    
    if (strcmp(args[0], "fileinfo") == 0) { 
        do_fileinfo(args); 
        if (teaching_mode) explain_command("fileinfo");
        return; 
    }
    if (strcmp(args[0], "hexdump") == 0) { 
        do_hexdump(args); 
        if (teaching_mode) explain_command("hexdump");
        return; 
    }
    if (strcmp(args[0], "duplicate") == 0) { 
        do_duplicate(args); 
        if (teaching_mode) explain_command("duplicate");
        return; 
    }
    if (strcmp(args[0], "encrypt") == 0) { 
        do_encrypt(args); 
        return; 
    }
    if (strcmp(args[0], "decrypt") == 0) { 
        do_decrypt(args); 
        return; 
    }
    if (strcmp(args[0], "sizeof") == 0) { 
        do_sizeof(args); 
        return; 
    }
    if (strcmp(args[0], "age") == 0) { 
        do_age(args); 
        return; 
    }
    if (strcmp(args[0], "freq") == 0) { 
        do_freq(args); 
        return; 
    }
    if (strcmp(args[0], "lines") == 0) { 
        do_lines(args); 
        return; 
    }
    if (strcmp(args[0], "quicknote") == 0) { 
        do_quicknote(args); 
        return; 
    }
    if (strcmp(args[0], "calc") == 0) { 
        do_calc(args); 
        return; 
    }
    if (strcmp(args[0], "head") == 0) { 
        do_head(args); 
        return; 
    }
    if (strcmp(args[0], "tail") == 0) { 
        do_tail(args); 
        return; 
    }
    if (strcmp(args[0], "wc") == 0) { 
        do_wc(args); 
        return; 
    }
    if (strcmp(args[0], "grep") == 0) { 
        do_grep(args); 
        return; 
    }
    if (strcmp(args[0], "sort") == 0) { 
        do_sort(args); 
        return; 
    }
    if (strcmp(args[0], "uniq") == 0) { 
        do_uniq(args); 
        return; 
    }
    if (strcmp(args[0], "rev") == 0) { 
        do_rev(args); 
        return; 
    }
    if (strcmp(args[0], "clear") == 0 || strcmp(args[0], "cls") == 0) { 
        do_clear(args); 
        return; 
    }
    if (strcmp(args[0], "date") == 0) { 
        do_date(args); 
        return; 
    }
    if (strcmp(args[0], "whoami") == 0) { 
        do_whoami(args); 
        return; 
    }
    if (strcmp(args[0], "hostname") == 0) { 
        do_hostname(args); 
        return; 
    }
    if (strcmp(args[0], "uptime") == 0) { 
        do_uptime(args); 
        return; 
    }
    if (strcmp(args[0], "df") == 0) { 
        do_df(args); 
        return; 
    }
    if (strcmp(args[0], "ps") == 0) { 
        do_ps(args); 
        return; 
    }
    if (strcmp(args[0], "kill") == 0) { 
        do_kill(args); 
        return; 
    }
    if (strcmp(args[0], "uniquecmds") == 0) { 
        do_uniquecmds(args); 
        return; 
    }
    
    // ============ OS MONITORING COMMANDS ============
    
    if (strcmp(args[0], "cpuinfo") == 0) { 
        do_cpuinfo(args); 
        return; 
    }
    if (strcmp(args[0], "meminfo") == 0) { 
        do_meminfo(args); 
        return; 
    }
    if (strcmp(args[0], "swapinfo") == 0) { 
        do_swapinfo(args); 
        return; 
    }
    if (strcmp(args[0], "diskinfo") == 0) { 
        do_diskinfo(args); 
        return; 
    }
    if (strcmp(args[0], "proclist") == 0 || strcmp(args[0], "proc") == 0 || strcmp(args[0], "ps") == 0) { 
        do_proclist(args); 
        return; 
    }
    if (strcmp(args[0], "proctop") == 0) { 
        do_proctop(args); 
        return; 
    }
    if (strcmp(args[0], "prockill") == 0) { 
        do_prockill(args); 
        return; 
    }
    if (strcmp(args[0], "netinfo") == 0) { 
        do_netinfo(args); 
        return; 
    }
    if (strcmp(args[0], "netstat") == 0) { 
        do_netstat(args); 
        return; 
    }
    if (strcmp(args[0], "connections") == 0) { 
        do_connections(args); 
        return; 
    }
    if (strcmp(args[0], "paging") == 0) { 
        do_paging(args); 
        return; 
    }
    if (strcmp(args[0], "vmstat") == 0) { 
        do_vmstat(args); 
        return; 
    }
    if (strcmp(args[0], "zoneinfo") == 0) { 
        do_zoneinfo(args); 
        return; 
    }
    if (strcmp(args[0], "loadavg") == 0) { 
        do_loadavg(args); 
        return; 
    }
    if (strcmp(args[0], "kernelinfo") == 0) { 
        do_kernelinfo(args); 
        return; 
    }
    if (strcmp(args[0], "filesystems") == 0) { 
        do_filesystems(args); 
        return; 
    }
    if (strcmp(args[0], "mounts") == 0) { 
        do_mounts(args); 
        return; 
    }
    if (strcmp(args[0], "modules") == 0) { 
        do_modules(args); 
        return; 
    }
    if (strcmp(args[0], "battery") == 0) { 
        do_battery(args); 
        return; 
    }
    if (strcmp(args[0], "sensors") == 0) { 
        do_sensors(args); 
        return; 
    }
    if (strcmp(args[0], "interrupts") == 0) { 
        do_interrupts(args); 
        return; 
    }
    if (strcmp(args[0], "users") == 0) { 
        do_users(args); 
        return; 
    }
    if (strcmp(args[0], "envvar") == 0) { 
        do_envvar(args); 
        return; 
    }
    if (strcmp(args[0], "openfiles") == 0) { 
        do_openfiles(args); 
        return; 
    }
    if (strcmp(args[0], "sockets") == 0) { 
        do_sockets(args); 
        return; 
    }
    
    // OS help command
    if (strcmp(args[0], "oshelp") == 0) {
        printf("\n\033[1;36m=== OS MONITORING COMMANDS ===\033[0m\n\n");
        printf("CPU & Memory:\n");
        printf("  cpuinfo      - CPU information and usage\n");
        printf("  meminfo      - Memory usage details\n");
        printf("  swapinfo     - Swap memory info\n");
        printf("\nDisk:\n");
        printf("  diskinfo     - Disk partitions and usage\n");
        printf("\nProcesses:\n");
        printf("  proclist     - List running processes\n");
        printf("  proctop [n]  - Top N processes by memory\n");
        printf("  prockill PID - Kill a process\n");
        printf("\nNetwork:\n");
        printf("  netinfo      - Network interfaces\n");
        printf("  netstat      - Network statistics\n");
        printf("  connections  - Active network connections\n");
        printf("\nPaging & Memory:\n");
        printf("  paging       - Paging statistics from /proc/vmstat\n");
        printf("  vmstat [n]   - Virtual memory stats (first n lines)\n");
        printf("  zoneinfo     - Memory zone information\n");
        printf("\nSystem:\n");
        printf("  loadavg      - System load averages\n");
        printf("  kernelinfo   - Kernel version\n");
        printf("  filesystems  - Supported filesystems\n");
        printf("  mounts       - Mounted filesystems\n");
        printf("  modules [n]  - Loaded kernel modules\n");
        printf("\nHardware:\n");
        printf("  battery      - Battery status\n");
        printf("  sensors      - Temperature sensors\n");
        printf("  interrupts   - Interrupt statistics\n");
        printf("\nUsers & Environment:\n");
        printf("  users        - Logged in users\n");
        printf("  envvar [name]- Environment variables\n");
        printf("  openfiles    - Open file descriptors\n");
        printf("  sockets      - Socket statistics\n");
        printf("\n");
        return;
    }
    
    // ============ IPC COMMANDS (Inter-Process Communication) ============
    
    if (strcmp(args[0], "mypid") == 0) {
        do_mypid(args);
        return;
    }
    if (strcmp(args[0], "terminals") == 0) {
        do_terminals(args);
        return;
    }
    if (strcmp(args[0], "sendmsg") == 0) {
        do_sendmsg(args);
        return;
    }
    if (strcmp(args[0], "broadcast") == 0) {
        do_broadcast(args);
        return;
    }
    if (strcmp(args[0], "checkmsg") == 0) {
        do_checkmsg(args);
        return;
    }
    if (strcmp(args[0], "readmsg") == 0) {
        do_readmsg(args);
        return;
    }
    if (strcmp(args[0], "clearmsg") == 0) {
        do_clearmsg(args);
        return;
    }
    if (strcmp(args[0], "ipchelp") == 0) {
        do_ipchelp(args);
        return;
    }
    if (strcmp(args[0], "register") == 0) {
        do_register_terminal(args);
        return;
    }
    if (strcmp(args[0], "unregister") == 0) {
        do_unregister_terminal(args);
        return;
    }
    
    // ============ SYSTEM MONITOR ============
    
    if (strcmp(args[0], "sysmon") == 0) {
        if (args[1] && strcmp(args[1], "-c") == 0) {
            sysmon_display_compact();
        } else if (args[1] && strcmp(args[1], "-l") == 0) {
            int dur = args[2] ? atoi(args[2]) : 10;
            sysmon_display_live(dur > 0 ? dur : 10);
        } else {
            sysmon_display_full();
        }
        if (teaching_mode) explain_command("sysmon");
        return;
    }
    
    // ============ VISUALIZER COMMANDS ============
    
    if (strcmp(args[0], "visualize") == 0) {
        do_visualize(args, history, undo_stack, trie, bktree);
        return;
    }
    
    // ============ EXPRESSION COMMANDS ============
    
    if (strcmp(args[0], "infix2postfix") == 0) {
        do_infix2postfix(args);
        return;
    }
    if (strcmp(args[0], "infix2prefix") == 0) {
        do_infix2prefix(args);
        return;
    }
    if (strcmp(args[0], "postfix2infix") == 0) {
        do_postfix2infix(args);
        return;
    }
    if (strcmp(args[0], "prefix2infix") == 0) {
        do_prefix2infix(args);
        return;
    }
    if (strcmp(args[0], "evaluate") == 0) {
        do_evaluate(args);
        return;
    }
    if (strcmp(args[0], "exprtype") == 0) {
        do_exprtype(args);
        return;
    }
    
    // Visualized expression commands
    if (strcmp(args[0], "visualfix") == 0) {
        do_infix2postfix_visual(args);
        return;
    }
    if (strcmp(args[0], "visualpost") == 0) {
        do_postfix2infix_visual(args);
        return;
    }
    if (strcmp(args[0], "visualeval") == 0) {
        do_evaluate_visual(args);
        return;
    }
    
    // ============ ENCODING/DECODING UTILITIES ============
    
    if (strcmp(args[0], "base64") == 0) {
        do_base64(args);
        return;
    }
    if (strcmp(args[0], "morse") == 0) {
        do_morse(args);
        return;
    }
    if (strcmp(args[0], "rot13") == 0) {
        do_rot13(args);
        return;
    }
    if (strcmp(args[0], "reverse") == 0) {
        do_reverse_text(args);
        return;
    }
    
    // ============ NUMBER CONVERSION UTILITIES ============
    
    if (strcmp(args[0], "binary") == 0) {
        do_binary(args);
        return;
    }
    if (strcmp(args[0], "hex") == 0) {
        do_hex(args);
        return;
    }
    if (strcmp(args[0], "octal") == 0) {
        do_octal(args);
        return;
    }
    if (strcmp(args[0], "baseconv") == 0) {
        do_base_convert(args);
        return;
    }
    
    // ============ FILE UTILITIES ============
    
    if (strcmp(args[0], "checksum") == 0) {
        do_checksum(args);
        return;
    }
    if (strcmp(args[0], "crc32") == 0) {
        do_crc32(args);
        return;
    }
    if (strcmp(args[0], "compress") == 0) {
        do_compress(args);
        return;
    }
    if (strcmp(args[0], "decompress") == 0) {
        do_decompress(args);
        return;
    }
    if (strcmp(args[0], "diff") == 0) {
        do_diff(args);
        return;
    }
    
    // ============ MISC UTILITIES ============
    
    if (strcmp(args[0], "random") == 0) {
        do_random(args);
        return;
    }
    if (strcmp(args[0], "uuid") == 0) {
        do_uuid(args);
        return;
    }
    if (strcmp(args[0], "timer") == 0) {
        do_timer(args);
        return;
    }
    if (strcmp(args[0], "stopwatch") == 0) {
        do_stopwatch(args);
        return;
    }
    if (strcmp(args[0], "colorpalette") == 0) {
        do_color_palette(args);
        return;
    }
    if (strcmp(args[0], "ascii") == 0) {
        do_ascii_art(args);
        return;
    }
    if (strcmp(args[0], "progress") == 0) {
        do_progress(args);
        return;
    }
    if (strcmp(args[0], "lorem") == 0) {
        do_lorem(args);
        return;
    }
    if (strcmp(args[0], "table") == 0) {
        do_table(args);
        return;
    }
    if (strcmp(args[0], "jsonformat") == 0) {
        do_json_format(args);
        return;
    }
    
    // ============ CD COMMAND ============
    
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            // Go to home directory
            char *home = getenv("HOME");
            if (home) {
                chdir(home);
            }
        } else if (strcmp(args[1], "~") == 0) {
            char *home = getenv("HOME");
            if (home) {
                chdir(home);
            }
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd");
            }
        }
        if (teaching_mode) explain_command("cd");
        return;
    }
    
    // ============ EXTERNAL COMMAND EXECUTION ============
    
    pid_t pid = fork();
    int status;
    
    if (pid == 0) {
        // Child process
        execvp(args[0], args);
        // If execvp returns, command not found
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("fork");
        return;
    } else {
        // Parent process
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_FAILURE) {
            status = -1;
        } else {
            status = 0;
        }
    }
    
    if (status == -1) {
        printf("Command not found: %s\n", args[0]);
        
        // Suggest corrections
        SuggestionList suggestions;
        suggestion_get_commands(args[0], &suggestions);
        
        if (suggestions.count > 0) {
            printf("Did you mean: ");
            for (int i = 0; i < suggestions.count && i < 3; i++) {
                printf("%s", suggestions.suggestions[i]);
                if (i < suggestions.count - 1 && i < 2) printf(", ");
            }
            printf("?\n");
        }
    } else {
        push_undo(undo_stack, cmd, UNDO_UNKNOWN, NULL, NULL);
        if (teaching_mode) {
            explain_command(args[0]);
        }
    }
}

// ============ MAIN FUNCTION ============

int main(int argc, char *argv[]) {
    char cmd[MAX_CMD_LEN];
    
    // Disable buffering for IPC with Python frontend
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    
    // Initialize data structures
    History *history = init_history(100);
    TrieNode *trie = create_node();
    BKTreeNode *bktree = NULL;
    UndoStack *undo_stack = init_undo_stack();
    init_macros();
    
    // Initialize NLP and suggestion engines
    nlp_init();
    suggestion_init();
    
    // Populate trie and bktree with all commands
    const char *commands[] = {
        // Basic file operations
        "ls", "pwd", "cd", "mkdir", "rmdir", "touch", "rm", "cat", "cp", "mv",
        "echo", "tree", "search", "backup", "compare", "stats", "sysmon",
        "bookmark", "recent", "bulk_rename", "help", "history", "exit", "clear",
        // Custom file commands
        "fileinfo", "hexdump", "duplicate", "encrypt", "decrypt", "sizeof",
        "age", "freq", "lines", "quicknote", "calc", "head", "tail", "wc",
        "grep", "sort", "uniq", "rev", "date", "whoami", "hostname", "uptime",
        "df", "ps", "kill", "undo", "macro", "teach",
        // Visualizer commands
        "visualize",
        // Expression commands
        "infix2postfix", "infix2prefix", "postfix2infix", "prefix2infix",
        "evaluate", "exprtype", "visualfix", "visualpost", "visualeval",
        // Utility commands
        "base64", "morse", "rot13", "reverse", "binary", "hex", "octal",
        "baseconv", "checksum", "crc32", "compress", "decompress", "diff",
        "random", "uuid", "timer", "stopwatch", "colorpalette", "ascii",
        "progress", "lorem", "table", "jsonformat"
    };
    int num_commands = sizeof(commands) / sizeof(commands[0]);
    
    for (int i = 0; i < num_commands; i++) {
        insert_trie(trie, commands[i]);
        insert_bktree(&bktree, commands[i]);
    }
    
    // Check for batch mode
    int batch_mode = (argc > 1 && strcmp(argv[1], "-c") == 0);
    
    if (batch_mode && argc > 2) {
        // Execute single command from argument
        strcpy(cmd, argv[2]);
        // Don't add !n commands to history
        if (!(cmd[0] == '!' && cmd[1] >= '0' && cmd[1] <= '9')) {
            add_history(history, cmd);
        }
        execute_line(cmd, history, trie, bktree, undo_stack);
    } else {
        // Interactive mode
        while (1) {
            type_prompt();
            read_command(cmd);
            
            if (strlen(cmd) == 0) continue;
            
            // Don't add !n commands to history
            if (!(cmd[0] == '!' && cmd[1] >= '0' && cmd[1] <= '9')) {
                add_history(history, cmd);
            }
            execute_line(cmd, history, trie, bktree, undo_stack);
        }
    }
    
    // Cleanup
    free_history(history);
    free_bktree(bktree);
    free_undo_stack(undo_stack);
    free_macros();
    
    return 0;
}
