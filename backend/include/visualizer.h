/**
 * Data Structure Visualizer
 * Visualizes internal data structures (linked lists, trees, stacks, etc.)
 */

#ifndef VISUALIZER_H
#define VISUALIZER_H

#include "macros.h"
#include "history.h"
#include "undo.h"
#include "trie.h"
#include "bktree.h"

// Visualize macro as linked list
void visualize_macro(const char *name);

// Visualize command history
void visualize_history(History *history);

// Visualize undo stack
void visualize_undo_stack(UndoStack *stack);

// Visualize trie structure (subset for readability)
void visualize_trie(TrieNode *root, int max_depth);

// Visualize BK-tree structure
void visualize_bktree(BKTreeNode *root, int max_depth);

// Generic visualizer command dispatcher
void do_visualize(char **args, History *history, UndoStack *undo_stack, TrieNode *trie, BKTreeNode *bktree);

#endif
