#ifndef HISTORY_H
#define HISTORY_H

typedef struct HistoryNode {
    char *command;
    struct HistoryNode *next;
    struct HistoryNode *prev;
} HistoryNode;

typedef struct {
    HistoryNode *head;
    HistoryNode *tail;
    int count;
    int capacity;
    char **array; // Dynamic array for fast access by index
} History;

History *init_history(int capacity);
void add_history(History *history, const char *command);
const char *get_history_at(History *history, int index);
void free_history(History *history);
void print_history(History *history);

#endif
