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

void parse_command(char *cmd, char **args) {
    int i = 0;
    char *token = strtok(cmd, " ");
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " ");
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
    printf("╔══════════════════════════════════════════════════════════════════════╗\n");
    printf("║                    NLP TERMINAL - COMMAND REFERENCE                  ║\n");
    printf("╚══════════════════════════════════════════════════════════════════════╝\n\n");
    
    printf("┌─ File Operations ───────────────────────────────────────────────────┐\n");
    printf("│ ls [path]          - List directory contents                        │\n");
    printf("│ pwd                - Print current directory                        │\n");
    printf("│ cd <path>          - Change directory                               │\n");
    printf("│ mkdir <name>       - Create directory                               │\n");
    printf("│ rmdir <name>       - Remove empty directory                         │\n");
    printf("│ touch <file>       - Create file                                    │\n");
    printf("│ rm <file>          - Remove file                                    │\n");
    printf("│ cat <file>         - Display file contents                          │\n");
    printf("│ cp <src> <dst>     - Copy file                                      │\n");
    printf("│ mv <src> <dst>     - Move/rename file                               │\n");
    printf("│ echo <text>        - Print text                                     │\n");
    printf("└─────────────────────────────────────────────────────────────────────┘\n\n");
    
    printf("┌─ Advanced File Operations ──────────────────────────────────────────┐\n");
    printf("│ tree [path]        - Directory tree view                            │\n");
    printf("│ search <pattern>   - Search in files                                │\n");
    printf("│ backup <file>      - Create timestamped backup                      │\n");
    printf("│ compare <f1> <f2>  - Compare two files                              │\n");
    printf("│ head <file> [n]    - Show first n lines                             │\n");
    printf("│ tail <file> [n]    - Show last n lines                              │\n");
    printf("│ wc <file>          - Word/line/char count                           │\n");
    printf("│ grep <pat> <file>  - Search pattern in file                         │\n");
    printf("│ sort <file> [-r]   - Sort lines                                     │\n");
    printf("│ uniq <file>        - Remove duplicate lines                         │\n");
    printf("│ rev <file>         - Reverse lines                                  │\n");
    printf("└─────────────────────────────────────────────────────────────────────┘\n\n");
    
    printf("┌─ Unique Commands (Not in standard UNIX) ────────────────────────────┐\n");
    printf("│ fileinfo <file>    - Detailed file info (size, hash, permissions)   │\n");
    printf("│ hexdump <file>     - Hex view of file contents                      │\n");
    printf("│ duplicate [path]   - Find duplicate files by content                │\n");
    printf("│ encrypt <f> <key>  - Encrypt file with XOR cipher                   │\n");
    printf("│ decrypt <f> <key>  - Decrypt file                                   │\n");
    printf("│ sizeof <pattern>   - Total size of matching files                   │\n");
    printf("│ age <days> [o|n]   - Find files older/newer than days               │\n");
    printf("│ freq <file> [n]    - Word frequency analysis                        │\n");
    printf("│ lines <file>       - Detailed line/word/char statistics             │\n");
    printf("│ quicknote          - Quick note taking (add/list/search/clear)      │\n");
    printf("│ calc <expr>        - Calculator (supports +,-,*,/,^)                │\n");
    printf("└─────────────────────────────────────────────────────────────────────┘\n\n");
    
    printf("┌─ System Information ─────────────────────────────────────────────────┐\n");
    printf("│ sysmon             - Full system resource monitor                   │\n");
    printf("│ sysmon -c          - Compact system info                            │\n");
    printf("│ sysmon -l          - Live updating monitor                          │\n");
    printf("│ ps                 - List running processes                         │\n");
    printf("│ kill <pid> [sig]   - Kill process                                   │\n");
    printf("│ df                 - Disk free space                                │\n");
    printf("│ uptime             - System uptime                                  │\n");
    printf("│ date               - Current date/time                              │\n");
    printf("│ whoami             - Current user                                   │\n");
    printf("│ hostname           - System hostname                                │\n");
    printf("└─────────────────────────────────────────────────────────────────────┘\n\n");
    
    printf("┌─ Shell Features ────────────────────────────────────────────────────┐\n");
    printf("│ history            - Show command history                           │\n");
    printf("│ bookmark [n] [p]   - Manage directory bookmarks                     │\n");
    printf("│ recent             - Recently modified files                        │\n");
    printf("│ bulk_rename <p><r> - Rename multiple files                          │\n");
    printf("│ stats              - Shell statistics                               │\n");
    printf("│ undo               - Undo last command                              │\n");
    printf("│ macro              - Macro recording (define/run/list)              │\n");
    printf("│ teach [on|off]     - Teaching mode                                  │\n");
    printf("│ clear              - Clear screen                                   │\n");
    printf("│ exit               - Exit shell                                     │\n");
    printf("└─────────────────────────────────────────────────────────────────────┘\n\n");
    
    printf("┌─ Natural Language Examples ─────────────────────────────────────────┐\n");
    printf("│ \"show all files\"            → ls                                    │\n");
    printf("│ \"create folder called test\" → mkdir test                            │\n");
    printf("│ \"what's in myfile.txt\"      → cat myfile.txt                        │\n");
    printf("│ \"go to home\"                → cd ~                                  │\n");
    printf("│ \"system monitor\"            → sysmon                                │\n");
    printf("│ \"find duplicates\"           → duplicate                             │\n");
    printf("│ \"calculate 2+2*3\"           → calc 2+2*3                            │\n");
    printf("└─────────────────────────────────────────────────────────────────────┘\n\n");
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
    
    if (strncmp(cmd, "NLP:", 4) == 0) {
        char translated[MAX_CMD_LEN];
        process_nlp_command(cmd + 4, translated);
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
    
    // History command
    if (strcmp(cmd, "history") == 0) {
        print_history(history);
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
    
    // Macro handling
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
            printf("Macros: Use 'macro record <name>' and 'macro stop' to manage\n");
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
        "ls", "pwd", "cd", "mkdir", "rmdir", "touch", "rm", "cat", "cp", "mv",
        "echo", "tree", "search", "backup", "compare", "stats", "sysmon",
        "bookmark", "recent", "bulk_rename", "help", "history", "exit", "clear",
        "fileinfo", "hexdump", "duplicate", "encrypt", "decrypt", "sizeof",
        "age", "freq", "lines", "quicknote", "calc", "head", "tail", "wc",
        "grep", "sort", "uniq", "rev", "date", "whoami", "hostname", "uptime",
        "df", "ps", "kill", "undo", "macro", "teach"
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
        add_history(history, cmd);
        execute_line(cmd, history, trie, bktree, undo_stack);
    } else {
        // Interactive mode
        while (1) {
            type_prompt();
            read_command(cmd);
            
            if (strlen(cmd) == 0) continue;
            
            add_history(history, cmd);
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
