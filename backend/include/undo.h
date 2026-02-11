#ifndef UNDO_H
#define UNDO_H

typedef enum {
    UNDO_MKDIR,
    UNDO_RMDIR,
    UNDO_TOUCH,
    UNDO_RM,
    UNDO_CP,
    UNDO_MV,
    UNDO_UNKNOWN
} UndoType;

typedef struct UndoNode {
    char *command;
    UndoType type;
    char *target;      // File/dir that was affected
    char *backup_path; // For files that were deleted/modified
    struct UndoNode *next;
} UndoNode;

typedef struct {
    UndoNode *top;
    int count;
} UndoStack;

UndoStack *init_undo_stack();
void push_undo(UndoStack *stack, const char *command, UndoType type, const char *target, const char *backup);
int execute_undo(UndoStack *stack);
void free_undo_stack(UndoStack *stack);

#endif
