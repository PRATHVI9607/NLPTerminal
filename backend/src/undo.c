#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "undo.h"
#include "utils.h"

UndoStack *init_undo_stack() {
    UndoStack *stack = malloc(sizeof(UndoStack));
    stack->top = NULL;
    stack->count = 0;
    return stack;
}

void push_undo(UndoStack *stack, const char *command) {
    UndoNode *node = malloc(sizeof(UndoNode));
    node->command = strdup_custom(command);
    node->next = stack->top;
    stack->top = node;
    stack->count++;
}

char *pop_undo(UndoStack *stack) {
    if (!stack->top) return NULL;
    
    UndoNode *node = stack->top;
    char *command = node->command;
    stack->top = node->next;
    stack->count--;
    
    free(node);
    return command;
}

void free_undo_stack(UndoStack *stack) {
    while (stack->top) {
        char *cmd = pop_undo(stack);
        free(cmd);
    }
    free(stack);
}
