#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "history.h"
#include "utils.h"

History *init_history(int capacity) {
    History *h = malloc(sizeof(History));
    h->head = NULL;
    h->tail = NULL;
    h->count = 0;
    h->capacity = capacity;
    h->array = malloc(sizeof(char*) * capacity);
    return h;
}

void add_history(History *history, const char *command) {
    if (!command || strlen(command) == 0) return;

    // Add to linked list
    HistoryNode *node = malloc(sizeof(HistoryNode));
    node->command = strdup_custom(command);
    node->next = NULL;
    node->prev = history->tail;

    if (history->tail) {
        history->tail->next = node;
    } else {
        history->head = node;
    }
    history->tail = node;

    // Add to array (resize if needed, for now just circular or simple append)
    // For simplicity in this step, we'll just realloc if needed or cap it.
    // Let's realloc.
    if (history->count >= history->capacity) {
        history->capacity *= 2;
        history->array = realloc(history->array, sizeof(char*) * history->capacity);
    }
    history->array[history->count] = node->command; // Point to the same string
    history->count++;
}

const char *get_history_at(History *history, int index) {
    if (index < 0 || index >= history->count) return NULL;
    return history->array[index];
}

void print_history(History *history) {
    for (int i = 0; i < history->count; i++) {
        printf("%d: %s\n", i + 1, history->array[i]);
    }
}

void free_history(History *history) {
    HistoryNode *current = history->head;
    while (current) {
        HistoryNode *next = current->next;
        free(current->command);
        free(current);
        current = next;
    }
    free(history->array);
    free(history);
}
