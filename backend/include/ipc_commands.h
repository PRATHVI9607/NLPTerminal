#ifndef IPC_COMMANDS_H
#define IPC_COMMANDS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

// IPC Message structure
#define IPC_MSG_DIR "/tmp/nlpterminal_ipc"
#define MAX_MSG_LEN 4096

// Initialize IPC system
void do_ipc_init(char **args);

// Send message to another terminal by PID
void do_sendmsg(char **args);

// Check for incoming messages
void do_checkmsg(char **args);

// Read and display incoming messages
void do_readmsg(char **args);

// Clear all messages
void do_clearmsg(char **args);

// List all active terminals
void do_terminals(char **args);

// Register this terminal
void do_register_terminal(char **args);

// Unregister this terminal
void do_unregister_terminal(char **args);

// Get current terminal's PID
void do_mypid(char **args);

// Broadcast message to all terminals
void do_broadcast(char **args);

// IPC Help
void do_ipchelp(char **args);

#endif // IPC_COMMANDS_H
