#ifndef UNDO_H
#define UNDO_H

typedef struct UndoNode {
    char *command;
    struct UndoNode *next;
} UndoNode;

typedef struct {
    UndoNode *top;
    int count;
} UndoStack;

UndoStack *init_undo_stack();
void push_undo(UndoStack *stack, const char *command);
char *pop_undo(UndoStack *stack);
void free_undo_stack(UndoStack *stack);

#endif
