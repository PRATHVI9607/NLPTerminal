#include "../include/ipc_commands.h"
#include <dirent.h>
#include <sys/file.h>

#define IPC_MSG_DIR "/tmp/nlpterminal_ipc"
#define IPC_TERMINALS_FILE "/tmp/nlpterminal_ipc/terminals.list"
#define MAX_MSG_LEN 4096

// Ensure IPC directory exists
static void ensure_ipc_dir() {
    struct stat st = {0};
    if (stat(IPC_MSG_DIR, &st) == -1) {
        mkdir(IPC_MSG_DIR, 0777);
    }
}

// Get inbox path for a PID
static void get_inbox_path(pid_t pid, char *path, size_t len) {
    snprintf(path, len, "%s/inbox_%d", IPC_MSG_DIR, pid);
}

// Initialize IPC system
void do_ipc_init(char **args) {
    (void)args;
    ensure_ipc_dir();
    
    // Create inbox for this terminal
    char inbox[256];
    get_inbox_path(getpid(), inbox, sizeof(inbox));
    
    // Create empty inbox file
    FILE *f = fopen(inbox, "a");
    if (f) fclose(f);
    
    printf("IPC initialized for terminal PID %d\n", getpid());
}

// Register terminal in the terminals list
void do_register_terminal(char **args) {
    (void)args;
    ensure_ipc_dir();
    
    pid_t pid = getpid();
    char inbox[256];
    get_inbox_path(pid, inbox, sizeof(inbox));
    
    // Create inbox
    FILE *f = fopen(inbox, "a");
    if (f) fclose(f);
    
    // Add to terminals list
    FILE *tlist = fopen(IPC_TERMINALS_FILE, "a+");
    if (!tlist) {
        perror("Cannot open terminals list");
        return;
    }
    
    // Check if already registered
    char line[64];
    int found = 0;
    while (fgets(line, sizeof(line), tlist)) {
        pid_t existing;
        if (sscanf(line, "%d", &existing) == 1 && existing == pid) {
            found = 1;
            break;
        }
    }
    
    if (!found) {
        fprintf(tlist, "%d %ld\n", pid, time(NULL));
    }
    fclose(tlist);
    
    printf("Terminal registered with PID %d\n", pid);
}

// Unregister terminal
void do_unregister_terminal(char **args) {
    (void)args;
    pid_t pid = getpid();
    
    // Remove from terminals list
    FILE *tlist = fopen(IPC_TERMINALS_FILE, "r");
    if (!tlist) return;
    
    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "%s/terminals.tmp", IPC_MSG_DIR);
    FILE *temp = fopen(temp_path, "w");
    if (!temp) {
        fclose(tlist);
        return;
    }
    
    char line[64];
    while (fgets(line, sizeof(line), tlist)) {
        pid_t existing;
        if (sscanf(line, "%d", &existing) == 1 && existing != pid) {
            fputs(line, temp);
        }
    }
    
    fclose(tlist);
    fclose(temp);
    rename(temp_path, IPC_TERMINALS_FILE);
    
    // Remove inbox
    char inbox[256];
    get_inbox_path(pid, inbox, sizeof(inbox));
    unlink(inbox);
    
    printf("Terminal unregistered\n");
}

// Get my PID
void do_mypid(char **args) {
    (void)args;
    printf("Terminal PID: %d\n", getpid());
}

// List all registered terminals
void do_terminals(char **args) {
    (void)args;
    ensure_ipc_dir();
    
    FILE *tlist = fopen(IPC_TERMINALS_FILE, "r");
    if (!tlist) {
        printf("No terminals registered yet.\n");
        return;
    }
    
    printf("\n\033[1;36m═══════════════════════════════════════\033[0m\n");
    printf("\033[1;33m       REGISTERED TERMINALS\033[0m\n");
    printf("\033[1;36m═══════════════════════════════════════\033[0m\n\n");
    
    char line[128];
    int count = 0;
    pid_t my_pid = getpid();
    
    while (fgets(line, sizeof(line), tlist)) {
        pid_t pid;
        long timestamp;
        if (sscanf(line, "%d %ld", &pid, &timestamp) >= 1) {
            // Check if process still exists
            if (kill(pid, 0) == 0) {
                char marker = (pid == my_pid) ? '*' : ' ';
                printf("  %c PID: \033[1;32m%d\033[0m", marker, pid);
                if (pid == my_pid) printf(" \033[1;33m(this terminal)\033[0m");
                printf("\n");
                count++;
            }
        }
    }
    
    fclose(tlist);
    
    if (count == 0) {
        printf("  No active terminals found.\n");
    }
    printf("\n  Total: %d terminal(s)\n", count);
    printf("\033[1;36m═══════════════════════════════════════\033[0m\n\n");
}

// Send message to another terminal
void do_sendmsg(char **args) {
    if (!args[1] || !args[2]) {
        fprintf(stderr, "Usage: sendmsg <PID> <message>\n");
        fprintf(stderr, "       sendmsg <tab_number> <message>\n");
        return;
    }
    
    pid_t target_pid = atoi(args[1]);
    
    // Check if target exists
    if (kill(target_pid, 0) != 0) {
        fprintf(stderr, "Error: Terminal with PID %d not found or not responding.\n", target_pid);
        return;
    }
    
    // Build message
    char message[MAX_MSG_LEN] = "";
    for (int i = 2; args[i]; i++) {
        if (strlen(message) + strlen(args[i]) + 2 < MAX_MSG_LEN) {
            strcat(message, args[i]);
            if (args[i+1]) strcat(message, " ");
        }
    }
    
    // Write to target's inbox
    char inbox[256];
    get_inbox_path(target_pid, inbox, sizeof(inbox));
    
    FILE *f = fopen(inbox, "a");
    if (!f) {
        fprintf(stderr, "Error: Cannot write to terminal %d's inbox.\n", target_pid);
        return;
    }
    
    // Lock file for writing
    flock(fileno(f), LOCK_EX);
    
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
    
    fprintf(f, "[%s] FROM PID %d: %s\n", time_str, getpid(), message);
    
    flock(fileno(f), LOCK_UN);
    fclose(f);
    
    // Note: GUI polls for messages, no signal needed
    
    printf("\033[1;32m✓\033[0m Message sent to PID %d\n", target_pid);
}

// Check for new messages (non-blocking)
void do_checkmsg(char **args) {
    (void)args;
    
    char inbox[256];
    get_inbox_path(getpid(), inbox, sizeof(inbox));
    
    FILE *f = fopen(inbox, "r");
    if (!f) {
        printf("No messages.\n");
        return;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);
    
    if (size == 0) {
        printf("No messages.\n");
    } else {
        printf("\033[1;33m📬 You have new messages! Use 'readmsg' to view them.\033[0m\n");
    }
}

// Read all messages
void do_readmsg(char **args) {
    (void)args;
    
    char inbox[256];
    get_inbox_path(getpid(), inbox, sizeof(inbox));
    
    FILE *f = fopen(inbox, "r");
    if (!f) {
        printf("No messages.\n");
        return;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    
    if (size == 0) {
        printf("No messages.\n");
        fclose(f);
        return;
    }
    
    fseek(f, 0, SEEK_SET);
    
    printf("\n\033[1;36m═══════════════════════════════════════\033[0m\n");
    printf("\033[1;33m           INBOX MESSAGES\033[0m\n");
    printf("\033[1;36m═══════════════════════════════════════\033[0m\n\n");
    
    char line[MAX_MSG_LEN];
    while (fgets(line, sizeof(line), f)) {
        printf("  \033[1;32m►\033[0m %s", line);
    }
    
    printf("\n\033[1;36m═══════════════════════════════════════\033[0m\n");
    printf("Use 'clearmsg' to clear all messages.\n\n");
    
    fclose(f);
}

// Clear all messages
void do_clearmsg(char **args) {
    (void)args;
    
    char inbox[256];
    get_inbox_path(getpid(), inbox, sizeof(inbox));
    
    FILE *f = fopen(inbox, "w");
    if (f) {
        fclose(f);
        printf("\033[1;32m✓\033[0m All messages cleared.\n");
    } else {
        perror("clearmsg");
    }
}

// Broadcast to all terminals
void do_broadcast(char **args) {
    if (!args[1]) {
        fprintf(stderr, "Usage: broadcast <message>\n");
        return;
    }
    
    // Build message
    char message[MAX_MSG_LEN] = "";
    for (int i = 1; args[i]; i++) {
        if (strlen(message) + strlen(args[i]) + 2 < MAX_MSG_LEN) {
            strcat(message, args[i]);
            if (args[i+1]) strcat(message, " ");
        }
    }
    
    FILE *tlist = fopen(IPC_TERMINALS_FILE, "r");
    if (!tlist) {
        printf("No terminals to broadcast to.\n");
        return;
    }
    
    pid_t my_pid = getpid();
    int sent = 0;
    char line[64];
    
    while (fgets(line, sizeof(line), tlist)) {
        pid_t pid;
        if (sscanf(line, "%d", &pid) == 1 && pid != my_pid) {
            if (kill(pid, 0) == 0) {
                // Build args for sendmsg
                char pid_str[16];
                snprintf(pid_str, sizeof(pid_str), "%d", pid);
                
                char inbox[256];
                get_inbox_path(pid, inbox, sizeof(inbox));
                
                FILE *f = fopen(inbox, "a");
                if (f) {
                    flock(fileno(f), LOCK_EX);
                    
                    time_t now = time(NULL);
                    struct tm *tm_info = localtime(&now);
                    char time_str[32];
                    strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
                    
                    fprintf(f, "[%s] BROADCAST from PID %d: %s\n", time_str, my_pid, message);
                    
                    flock(fileno(f), LOCK_UN);
                    fclose(f);
                    
                    kill(pid, SIGUSR1);
                    sent++;
                }
            }
        }
    }
    
    fclose(tlist);
    printf("\033[1;32m✓\033[0m Broadcast sent to %d terminal(s)\n", sent);
}

// IPC Help
void do_ipchelp(char **args) {
    (void)args;
    printf("\n\033[1;36m═══════════════════════════════════════════════════════════\033[0m\n");
    printf("\033[1;33m         INTER-PROCESS COMMUNICATION COMMANDS\033[0m\n");
    printf("\033[1;36m═══════════════════════════════════════════════════════════\033[0m\n\n");
    
    printf("  \033[1;32mmypid\033[0m              - Show this terminal's PID\n");
    printf("  \033[1;32mterminals\033[0m          - List all registered terminals\n");
    printf("  \033[1;32msendmsg <PID> <msg>\033[0m - Send message to another terminal\n");
    printf("  \033[1;32mbroadcast <msg>\033[0m    - Send message to all terminals\n");
    printf("  \033[1;32mcheckmsg\033[0m           - Check for new messages\n");
    printf("  \033[1;32mreadmsg\033[0m            - Read all messages\n");
    printf("  \033[1;32mclearmsg\033[0m           - Clear all messages\n\n");
    
    printf("\033[1;35mProcess Synchronization:\033[0m\n");
    printf("  Messages can be used to synchronize processes.\n");
    printf("  When a message arrives, you'll see a notification.\n");
    printf("  Use 'readmsg' to view without interrupting work.\n\n");
    
    printf("\033[1;36m═══════════════════════════════════════════════════════════\033[0m\n\n");
}
