#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "undo.h"
#include "utils.h"

#define UNDO_BACKUP_DIR ".shell_undo"

UndoStack *init_undo_stack() {
    UndoStack *stack = malloc(sizeof(UndoStack));
    stack->top = NULL;
    stack->count = 0;
    
    // Create undo backup directory if it doesn't exist
    mkdir(UNDO_BACKUP_DIR, 0755);
    
    return stack;
}

void push_undo(UndoStack *stack, const char *command, UndoType type, const char *target, const char *backup) {
    UndoNode *node = malloc(sizeof(UndoNode));
    node->command = strdup_custom(command);
    node->type = type;
    node->target = target ? strdup_custom(target) : NULL;
    node->backup_path = backup ? strdup_custom(backup) : NULL;
    node->next = stack->top;
    stack->top = node;
    stack->count++;
    
    // Limit stack size to prevent memory issues
    if (stack->count > 50) {
        // Remove oldest entry
        UndoNode *curr = stack->top;
        UndoNode *prev = NULL;
        while (curr->next) {
            prev = curr;
            curr = curr->next;
        }
        if (prev) {
            prev->next = NULL;
            if (curr->backup_path) {
                unlink(curr->backup_path);
                free(curr->backup_path);
            }
            free(curr->command);
            if (curr->target) free(curr->target);
            free(curr);
            stack->count--;
        }
    }
}

int execute_undo(UndoStack *stack) {
    if (!stack->top) {
        printf("Nothing to undo.\n");
        return 0;
    }
    
    UndoNode *node = stack->top;
    int success = 0;
    
    printf("Undoing: %s\n", node->command);
    
    switch (node->type) {
        case UNDO_MKDIR:
            // Remove the directory that was created
            if (node->target && rmdir(node->target) == 0) {
                printf("Removed directory: %s\n", node->target);
                success = 1;
            } else {
                perror("Failed to undo mkdir");
            }
            break;
            
        case UNDO_TOUCH:
        case UNDO_RM:
            // Restore file from backup
            if (node->backup_path && node->target) {
                if (rename(node->backup_path, node->target) == 0) {
                    printf("Restored file: %s\n", node->target);
                    success = 1;
                } else {
                    perror("Failed to restore file");
                }
            } else if (node->target) {
                // Just remove the file that was created
                if (unlink(node->target) == 0) {
                    printf("Removed file: %s\n", node->target);
                    success = 1;
                }
            }
            break;
            
        case UNDO_CP:
            // Remove the copied file
            if (node->target && unlink(node->target) == 0) {
                printf("Removed copied file: %s\n", node->target);
                success = 1;
            } else {
                perror("Failed to undo copy");
            }
            break;
            
        case UNDO_MV:
            // Restore from backup
            if (node->backup_path && node->target) {
                if (rename(node->backup_path, node->target) == 0) {
                    printf("Restored moved file: %s\n", node->target);
                    success = 1;
                } else {
                    perror("Failed to undo move");
                }
            }
            break;
            
        default:
            printf("Cannot undo this command type.\n");
            break;
    }
    
    // Pop from stack
    stack->top = node->next;
    stack->count--;
    
    if (node->backup_path) free(node->backup_path);
    if (node->target) free(node->target);
    free(node->command);
    free(node);
    
    return success;
}

void free_undo_stack(UndoStack *stack) {
    while (stack->top) {
        UndoNode *node = stack->top;
        stack->top = node->next;
        
        if (node->backup_path) {
            unlink(node->backup_path);
            free(node->backup_path);
        }
        if (node->target) free(node->target);
        free(node->command);
        free(node);
    }
    free(stack);
}
