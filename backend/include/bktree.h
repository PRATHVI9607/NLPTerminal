#ifndef BKTREE_H
#define BKTREE_H

typedef struct BKTreeNode {
    char *word;
    struct BKTreeNode *children; // Linked list of children (simplified)
    struct BKTreeNode *next; // Sibling
    int distance; // Edge weight from parent
} BKTreeNode;

BKTreeNode *create_bk_node(const char *word);
void insert_bktree(BKTreeNode **root, const char *word);
void get_similar_words(BKTreeNode *root, const char *query, int tolerance, char **results, int *count);
int levenshtein_distance(const char *s1, const char *s2);
void free_bktree(BKTreeNode *root);

#endif
