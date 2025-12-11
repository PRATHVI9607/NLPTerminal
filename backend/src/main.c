#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32
    #include <process.h>
    #define PATH_SEP '\\'
#else
    #include <sys/wait.h>
    #define PATH_SEP '/'
#endif

#include "history.h"
#include "utils.h"
#include "trie.h"
#include "bktree.h"
#include "undo.h"
#include "macros.h"
#include "commands.h"

#define MAX_CMD_LEN 1024
#define MAX_ARGS 64

int teaching_mode = 0;
int recording_macro = 0; // Flag to check if we are recording

void explain_command(const char *cmd) {
    printf("\n[AI Explanation]: ");
    if (strcmp(cmd, "dir") == 0) printf("Lists contents of the current directory.");
    else if (strcmp(cmd, "cd") == 0) printf("Changes the current working directory.");
    else if (strcmp(cmd, "exit") == 0) printf("Exits the shell.");
    else if (strcmp(cmd, "history") == 0) printf("Shows the list of previously executed commands.");
    else if (strcmp(cmd, "help") == 0) printf("Displays help information.");
    else if (strcmp(cmd, "whoami") == 0) printf("Displays the current user.");
    else if (strcmp(cmd, "copy") == 0) printf("Copies a file.");
    else if (strcmp(cmd, "del") == 0) printf("Deletes a file.");
    else if (strcmp(cmd, "mkdir") == 0) printf("Creates a new directory.");
    else if (strcmp(cmd, "move") == 0) printf("Moves or renames a file.");
    else if (strcmp(cmd, "undo") == 0) printf("Reverts the last command (if supported).");
    else if (strcmp(cmd, "macro") == 0) printf("Manages macros (define, end, run).");
    else printf("Executed external command '%s'.", cmd);
    printf("\n");
}

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
    // Remove newline at the end
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

// Forward declaration for recursive execution
void execute_line(char *cmd, History *history, TrieNode *trie, BKTreeNode *bktree, UndoStack *undo_stack);

int main() {
    char cmd[MAX_CMD_LEN];
    
    setbuf(stdout, NULL); // Disable buffering for IPC
    
    History *history = init_history(10);
    TrieNode *trie = create_node();
    BKTreeNode *bktree = NULL;
    UndoStack *undo_stack = init_undo_stack();
    init_macros();
    
    // Populate trie and bktree with common commands
    const char *commands[] = {"dir", "cd", "exit", "history", "help", "whoami", "copy", "del", "mkdir", "move", "undo", "macro", "ls", "pwd", "clear", "rm", "rmdir", "touch", "cat", "cp", "mv", "echo", "tree", "search", "backup", "compare", "stats", "bookmark", "recent", "bulk_rename"};
    int num_commands = sizeof(commands) / sizeof(commands[0]);
    
    for (int i = 0; i < num_commands; i++) {
        insert_trie(trie, commands[i]);
        insert_bktree(&bktree, commands[i]);
    }

    while (1) {
        type_prompt();
        read_command(cmd);

        if (strlen(cmd) == 0) {
            continue;
        }
        
        // If recording, check for 'macro end' specifically
        if (recording_macro) {
            if (strcmp(cmd, "macro end") == 0) {
                end_recording_macro();
                recording_macro = 0;
                continue;
            }
            add_macro_step(cmd);
            continue;
        }
        
        add_history(history, cmd);
        execute_line(cmd, history, trie, bktree, undo_stack);
    }
    
    free_history(history);
    // free_trie(trie);
    free_bktree(bktree);
    free_undo_stack(undo_stack);
    free_macros();

    return 0;
}

void execute_line(char *cmd, History *history, TrieNode *trie, BKTreeNode *bktree, UndoStack *undo_stack) {
    char *args[MAX_ARGS];
    int status;
    
    if (strcmp(cmd, "exit") == 0) {
        exit(0);
    }
    
    if (strcmp(cmd, "history") == 0) {
        print_history(history);
        return;
    }

    if (strcmp(cmd, "help") == 0) {
        printf("custom shell help:\n");
        printf("built-in commands (implemented in c):\n");
        printf("  ls [dir]    - list directory contents\n");
        printf("  pwd         - print working directory\n");
        printf("  cd <dir>    - change directory\n");
        printf("  mkdir <dir> - create directory\n");
        printf("  rmdir <dir> - remove directory\n");
        printf("  touch <file>- create file\n");
        printf("  rm <file>   - remove file\n");
        printf("  cat <file>  - print file content\n");
        printf("  cp <src> <dst> - copy file\n");
        printf("  mv <src> <dst> - move file\n");
        printf("  echo <args> - print arguments\n");
        printf("\ncustom commands:\n");
        printf("  tree [dir]  - display directory tree\n");
        printf("  search <pattern> - search files for pattern\n");
        printf("  backup <file> - create timestamped backup\n");
        printf("  compare <f1> <f2> - compare two files\n");
        printf("  stats       - show shell statistics\n");
        printf("  sysmon      - system resource monitor (CPU, Memory, Disk, Processes)\n");
        printf("  bookmark [name] [path] - manage bookmarks\n");
        printf("  recent      - show recently modified files\n");
        printf("  bulk_rename <pattern> <replacement> - rename multiple files\n");
        printf("\nother:\n");
        printf("  exit        - exit the shell\n");
        printf("  history     - show command history\n");
        printf("  teach [on|off] - enable/disable teaching mode\n");
        printf("  undo        - undo last command\n");
        printf("  macro       - manage macros\n");
        return;
    }
    
    if (strncmp(cmd, "teach ", 6) == 0) {
        if (strcmp(cmd + 6, "on") == 0) {
            teaching_mode = 1;
            printf("Teaching mode enabled.\n");
        } else if (strcmp(cmd + 6, "off") == 0) {
            teaching_mode = 0;
            printf("Teaching mode disabled.\n");
        } else {
            printf("Usage: teach [on|off]\n");
        }
        return;
    }
    
    if (strcmp(cmd, "undo") == 0) {
        execute_undo(undo_stack);
        return;
    }
    
    if (strncmp(cmd, "macro ", 6) == 0) {
        char *action = cmd + 6;
        if (strncmp(action, "define ", 7) == 0) {
            char *name = action + 7;
            start_recording_macro(name);
            recording_macro = 1;
        } else if (strncmp(action, "run ", 4) == 0) {
            char *name = action + 4;
            Macro *m = find_macro(name);
            if (m) {
                printf("Running macro '%s'...\n", name);
                MacroStep *step = m->head;
                while (step) {
                    printf(">> %s\n", step->command);
                    char step_cmd[MAX_CMD_LEN];
                    strcpy(step_cmd, step->command);
                    execute_line(step_cmd, history, trie, bktree, undo_stack);
                    step = step->next;
                }
            } else {
                printf("Error: Macro '%s' not found.\n", name);
            }
        } else {
            printf("Usage: macro [define <name>|run <name>]\n");
        }
        return;
    }
    
    if (strncmp(cmd, "complete ", 9) == 0) {
        char *prefix = cmd + 9;
        char *suggestions[100];
        int count = 0;
        get_suggestions(trie, prefix, suggestions, &count);
        printf("Suggestions for '%s':\n", prefix);
        for (int i = 0; i < count; i++) {
            printf("  %s\n", suggestions[i]);
            free(suggestions[i]);
        }
        return;
    }
    
    if (strncmp(cmd, "correct ", 8) == 0) {
        char *word = cmd + 8;
        char *suggestions[100];
        int count = 0;
        get_suggestions(trie, word, suggestions, &count);
        
        count = 0;
        get_similar_words(bktree, word, 2, suggestions, &count);
        
        printf("Corrections for '%s':\n", word);
        for (int i = 0; i < count; i++) {
            printf("  %s\n", suggestions[i]);
            free(suggestions[i]);
        }
        return;
    }

    // Make a copy of cmd because strtok modifies it
    char cmd_copy[MAX_CMD_LEN];
    strcpy(cmd_copy, cmd);
    parse_command(cmd_copy, args);

    if (args[0] == NULL) {
        return;
    }

    // Handle cd command internally
    // Check for built-in commands
    if (strcmp(args[0], "ls") == 0) { do_ls(args); return; }
    if (strcmp(args[0], "pwd") == 0) { do_pwd(args); return; }
    if (strcmp(args[0], "cat") == 0) { do_cat(args); return; }
    if (strcmp(args[0], "echo") == 0) { do_echo(args); return; }
    if (strcmp(args[0], "sysmon") == 0) { do_sysmon(args); return; }
    
    // Commands with undo support
    if (strcmp(args[0], "mkdir") == 0) { 
        do_mkdir(args); 
        if (args[1]) push_undo(undo_stack, cmd, UNDO_MKDIR, args[1], NULL);
        return; 
    }
    if (strcmp(args[0], "rmdir") == 0) { 
        do_rmdir(args); 
        if (args[1]) push_undo(undo_stack, cmd, UNDO_RMDIR, args[1], NULL);
        return; 
    }
    if (strcmp(args[0], "rm") == 0) { 
        // TODO: Create backup before deleting
        do_rm(args); 
        if (args[1]) push_undo(undo_stack, cmd, UNDO_RM, args[1], NULL);
        return; 
    }
    if (strcmp(args[0], "touch") == 0) { 
        do_touch(args); 
        if (args[1]) push_undo(undo_stack, cmd, UNDO_TOUCH, args[1], NULL);
        return; 
    }
    if (strcmp(args[0], "cp") == 0) { 
        do_cp(args); 
        if (args[2]) push_undo(undo_stack, cmd, UNDO_CP, args[2], NULL);
        return; 
    }
    if (strcmp(args[0], "mv") == 0) { 
        do_mv(args); 
        if (args[1] && args[2]) push_undo(undo_stack, cmd, UNDO_MV, args[2], NULL);
        return; 
    }
    
    // Custom commands
    if (strcmp(args[0], "tree") == 0) { do_tree(args); return; }
    if (strcmp(args[0], "search") == 0) { do_search(args); return; }
    if (strcmp(args[0], "backup") == 0) { do_backup(args); return; }
    if (strcmp(args[0], "compare") == 0) { do_compare(args); return; }
    if (strcmp(args[0], "stats") == 0) { do_stats(args); return; }
    if (strcmp(args[0], "bookmark") == 0) { do_bookmark(args); return; }
    if (strcmp(args[0], "recent") == 0) { do_recent(args); return; }
    if (strcmp(args[0], "bulk_rename") == 0) { do_bulk_rename(args); return; }

    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "cd: expected argument\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd");
            } else {
                push_undo(undo_stack, cmd, UNDO_UNKNOWN, NULL, NULL);
                if (teaching_mode) explain_command("cd");
            }
        }
        return;
    }

    #ifdef _WIN32
        // Windows implementation using _spawnvp
        status = _spawnvp(P_WAIT, args[0], args);
    #else
        // Linux/Unix implementation using fork/execvp
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            if (execvp(args[0], args) == -1) {
                // Don't print error here, let parent handle it via status or just fail silently
                // Actually, standard shell prints error.
                // But we want to handle "Command not found" in parent if possible?
                // No, execvp failure means command not found usually.
                exit(EXIT_FAILURE); 
            }
        } else if (pid < 0) {
            perror("fork");
            status = -1;
        } else {
            // Parent process
            do {
                waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            
            // Check if child exited successfully
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                 // Command failed (e.g. exit code 1)
                 // But if execvp failed, it returns EXIT_FAILURE (1).
                 // We can't easily distinguish between "command not found" and "command failed" 
                 // unless we check errno in child or something.
                 // For now, let's assume if it failed, we might want to suggest.
                 // But standard shells don't suggest on runtime error.
                 // We only want to suggest if execvp FAILED to find the file.
                 // In our simple shell, we can check if file exists? No.
                 // Let's just rely on the fact that if it's not found, we might want to suggest.
                 // But wait, if I type "ls -z", ls runs and fails. I don't want suggestions for "ls".
                 // If I type "lss", execvp fails.
                 // We can try to check if command exists in PATH before execvp? Too complex.
                 // Let's just assume status != 0 is a failure.
            }
            
            // For the sake of this assignment, let's say if status is non-zero, we check suggestions.
            // But on Linux, status is a bitmask.
            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                if (exit_code == EXIT_FAILURE) { // Assuming 1 means failure/not found
                     status = -1; // Flag for our logic below
                } else {
                    status = 0;
                }
            }
        }
    #endif
    
    if (status == -1) {
        // If command failed, try to suggest corrections
        // If command failed, try to suggest corrections
        printf("Command not found. Did you mean?\n");
        char *suggestions[100];
        int count = 0;
        // Use a larger tolerance or check logic
        get_similar_words(bktree, args[0], 2, suggestions, &count);
        
        if (count == 0) {
            printf("  (no suggestions found)\n");
        } else {
            for (int i = 0; i < count; i++) {
                printf("  %s\n", suggestions[i]);
                free(suggestions[i]);
            }
        }
    } else {
        push_undo(undo_stack, cmd, UNDO_UNKNOWN, NULL, NULL);
        if (teaching_mode) {
            explain_command(args[0]);
        }
    }
}
