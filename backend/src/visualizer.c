/**
 * Data Structure Visualizer
 * Clean ANSI-colored visualization of internal data structures
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "visualizer.h"
#include "macros.h"
#include "history.h"
#include "undo.h"
#include "trie.h"
#include "bktree.h"

// ANSI Color codes
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define DIM     "\033[2m"
#define CYAN    "\033[36m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define RED     "\033[31m"

// Visualize macro as linked list
void visualize_macro(const char *name) {
    Macro *macro = find_macro(name);
    
    if (!macro) {
        printf("%sError:%s Macro '%s' not found.\n", RED, RESET, name);
        return;
    }
    
    printf("\n%s%s=== MACRO LINKED LIST: %s ===%s\n\n", BOLD, CYAN, name, RESET);
    
    MacroStep *current = macro->head;
    int step_num = 1;
    
    if (!current) {
        printf("  %s(Empty Macro)%s\n\n", DIM, RESET);
        return;
    }
    
    printf("  %sHEAD%s\n", BOLD, RESET);
    printf("    |\n");
    
    while (current) {
        printf("    v\n");
        printf("  %s[Step %d]%s %s%s%s\n", YELLOW, step_num, RESET, GREEN, current->command, RESET);
        printf("    |  next: %s\n", current->next ? "-> (next node)" : "NULL");
        
        current = current->next;
        step_num++;
    }
    
    printf("    v\n");
    printf("  %sNULL%s (end)\n", DIM, RESET);
    printf("\n  %sTotal Steps:%s %d\n\n", BOLD, RESET, step_num - 1);
}

// Visualize command history
void visualize_history(History *history) {
    if (!history) {
        printf("%sError:%s History not available.\n", RED, RESET);
        return;
    }
    
    printf("\n%s%s=== COMMAND HISTORY ===%s\n", BOLD, CYAN, RESET);
    printf("  %sData Structure:%s Doubly Linked List + Array\n\n", DIM, RESET);
    
    printf("  Capacity: %s%d%s\n", YELLOW, history->capacity, RESET);
    printf("  Count:    %s%d%s\n\n", YELLOW, history->count, RESET);
    
    if (history->count == 0) {
        printf("  %s(Empty History)%s\n\n", DIM, RESET);
        return;
    }
    
    // Show as simple list
    printf("  %s%-5s  %-50s%s\n", BOLD, "IDX", "COMMAND", RESET);
    printf("  %-5s  %-50s\n", "---", "--------------------------------------------------");
    
    HistoryNode *node = history->head;
    int i = 1;
    while (node && i <= 10) {
        printf("  %s%-5d%s  %s\n", CYAN, i, RESET, 
               node->command ? node->command : "(empty)");
        node = node->next;
        i++;
    }
    
    if (node) {
        printf("  %s...    (%d more commands)%s\n", DIM, history->count - 10, RESET);
    }
    printf("\n");
}

// Visualize undo stack
void visualize_undo_stack(UndoStack *stack) {
    if (!stack) {
        printf("%sError:%s Undo stack not available.\n", RED, RESET);
        return;
    }
    
    printf("\n%s%s=== UNDO STACK ===%s\n", BOLD, CYAN, RESET);
    printf("  %sData Structure:%s Stack (LIFO)\n\n", DIM, RESET);
    
    printf("  Count: %s%d%s\n\n", YELLOW, stack->count, RESET);
    
    if (stack->count == 0 || !stack->top) {
        printf("  %s(Empty Stack)%s\n\n", DIM, RESET);
        return;
    }
    
    printf("  %sTOP%s\n", BOLD, RESET);
    printf("    |\n");
    
    UndoNode *current = stack->top;
    int index = 0;
    
    while (current) {
        const char *type_str;
        const char *type_color;
        
        switch (current->type) {
            case UNDO_MKDIR: type_str = "MKDIR"; type_color = GREEN; break;
            case UNDO_RMDIR: type_str = "RMDIR"; type_color = RED; break;
            case UNDO_TOUCH: type_str = "TOUCH"; type_color = GREEN; break;
            case UNDO_RM: type_str = "RM"; type_color = RED; break;
            case UNDO_CP: type_str = "CP"; type_color = BLUE; break;
            case UNDO_MV: type_str = "MV"; type_color = YELLOW; break;
            default: type_str = "UNKNOWN"; type_color = DIM; break;
        }
        
        printf("    v\n");
        printf("  %s[%d]%s %s%s%s", CYAN, index, RESET, type_color, type_str, RESET);
        if (current->target) {
            printf(" -> %s", current->target);
        }
        printf("\n");
        
        current = current->next;
        index++;
    }
    
    printf("    |\n");
    printf("  %sBOTTOM%s\n\n", DIM, RESET);
}

// Helper to print trie node recursively
static void print_trie_node(TrieNode *node, int depth, int max_depth, char *word, int pos) {
    if (!node || depth > max_depth) return;
    
    for (int i = 0; i < 26; i++) {
        if (node->children[i]) {
            char c = 'a' + i;
            word[pos] = c;
            word[pos + 1] = '\0';
            
            // Print indentation
            for (int j = 0; j < depth; j++) printf("  ");
            
            printf("%s%c%s", CYAN, c, RESET);
            
            if (node->children[i]->is_end_of_word) {
                printf(" %s[%s]%s", GREEN, word, RESET);
            }
            printf("\n");
            
            print_trie_node(node->children[i], depth + 1, max_depth, word, pos + 1);
        }
    }
}

// Visualize trie structure
void visualize_trie(TrieNode *root, int max_depth) {
    if (!root) {
        printf("%sError:%s Trie not available.\n", RED, RESET);
        return;
    }
    
    printf("\n%s%s=== COMMAND TRIE ===%s\n", BOLD, CYAN, RESET);
    printf("  %sData Structure:%s Trie (Prefix Tree)\n\n", DIM, RESET);
    
    printf("  %sROOT%s\n", BOLD, RESET);
    
    char word[256] = "";
    print_trie_node(root, 1, max_depth, word, 0);
    
    printf("\n  %s[word]%s = Complete command\n", GREEN, RESET);
    printf("  Depth shown: %d levels\n\n", max_depth);
}

// Helper for BK-tree visualization
static void print_bktree_node(BKTreeNode *node, int depth, int max_depth) {
    if (!node || depth > max_depth) return;
    
    // Print indentation
    for (int i = 0; i < depth; i++) printf("  ");
    
    printf("%s\"%s\"%s %s(d=%d)%s\n", YELLOW, node->word, RESET, DIM, node->distance, RESET);
    
    // Print children
    BKTreeNode *child = node->children;
    int count = 0;
    while (child && count < 5) {
        print_bktree_node(child, depth + 1, max_depth);
        child = child->next;
        count++;
    }
    
    if (child) {
        for (int i = 0; i <= depth; i++) printf("  ");
        printf("%s... (more)%s\n", DIM, RESET);
    }
}

// Visualize BK-tree
void visualize_bktree(BKTreeNode *root, int max_depth) {
    if (!root) {
        printf("%sError:%s BK-Tree not available.\n", RED, RESET);
        return;
    }
    
    printf("\n%s%s=== BK-TREE (Spell Checker) ===%s\n", BOLD, CYAN, RESET);
    printf("  %sData Structure:%s BK-Tree (Edit Distance)\n\n", DIM, RESET);
    
    printf("  %sROOT:%s %s\"%s\"%s\n", BOLD, RESET, YELLOW, root->word, RESET);
    
    BKTreeNode *child = root->children;
    while (child) {
        print_bktree_node(child, 1, max_depth);
        child = child->next;
    }
    
    printf("\n  %sd=N%s = Edit distance from parent\n", DIM, RESET);
    printf("  Depth shown: %d levels\n\n", max_depth);
}

// Main visualize command dispatcher
void do_visualize(char **args, History *history, UndoStack *undo_stack, TrieNode *trie, BKTreeNode *bktree) {
    if (!args[1]) {
        printf("\n%s%sVISUALIZE - Data Structure Viewer%s\n\n", BOLD, CYAN, RESET);
        printf("Usage: visualize <type> [args]\n\n");
        printf("Types:\n");
        printf("  %smacro%s <name>   - View macro as linked list\n", YELLOW, RESET);
        printf("  %shistory%s        - View command history\n", YELLOW, RESET);
        printf("  %sundo%s           - View undo stack\n", YELLOW, RESET);
        printf("  %strie%s [depth]   - View command trie (default: 3)\n", YELLOW, RESET);
        printf("  %sbktree%s [depth] - View BK-tree (default: 2)\n\n", YELLOW, RESET);
        return;
    }
    
    if (strcmp(args[1], "macro") == 0) {
        if (!args[2]) {
            printf("Usage: visualize macro <name>\n");
            return;
        }
        visualize_macro(args[2]);
    }
    else if (strcmp(args[1], "history") == 0) {
        visualize_history(history);
    }
    else if (strcmp(args[1], "undo") == 0) {
        visualize_undo_stack(undo_stack);
    }
    else if (strcmp(args[1], "trie") == 0) {
        int depth = args[2] ? atoi(args[2]) : 3;
        if (depth < 1) depth = 3;
        if (depth > 10) depth = 10;
        visualize_trie(trie, depth);
    }
    else if (strcmp(args[1], "bktree") == 0) {
        int depth = args[2] ? atoi(args[2]) : 2;
        if (depth < 1) depth = 2;
        if (depth > 5) depth = 5;
        visualize_bktree(bktree, depth);
    }
    else {
        printf("%sUnknown type:%s %s\n", RED, RESET, args[1]);
        printf("Run 'visualize' without args for help.\n");
    }
}
