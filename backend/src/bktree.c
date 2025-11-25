#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bktree.h"
#include "utils.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MIN3(a,b,c) (MIN(MIN(a,b),c))

int levenshtein_distance(const char *s1, const char *s2) {
    int len1 = strlen(s1);
    int len2 = strlen(s2);
    int matrix[len1 + 1][len2 + 1];
    int i, j;

    for (i = 0; i <= len1; i++) matrix[i][0] = i;
    for (j = 0; j <= len2; j++) matrix[0][j] = j;

    for (i = 1; i <= len1; i++) {
        for (j = 1; j <= len2; j++) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            matrix[i][j] = MIN3(
                matrix[i - 1][j] + 1,      // deletion
                matrix[i][j - 1] + 1,      // insertion
                matrix[i - 1][j - 1] + cost // substitution
            );
        }
    }
    return matrix[len1][len2];
}

BKTreeNode *create_bk_node(const char *word) {
    BKTreeNode *node = malloc(sizeof(BKTreeNode));
    node->word = strdup_custom(word);
    node->children = NULL;
    node->next = NULL;
    node->distance = 0;
    return node;
}

void insert_bktree(BKTreeNode **root, const char *word) {
    if (*root == NULL) {
        *root = create_bk_node(word);
        return;
    }

    BKTreeNode *curr = *root;
    int dist = levenshtein_distance(curr->word, word);
    
    // Find child with this distance
    BKTreeNode *child = curr->children;
    while (child) {
        if (child->distance == dist) {
            insert_bktree(&child, word); // Recurse
            return;
        }
        child = child->next;
    }

    // No child with this distance, create new child
    BKTreeNode *new_node = create_bk_node(word);
    new_node->distance = dist;
    new_node->next = curr->children;
    curr->children = new_node;
}

void get_similar_words(BKTreeNode *root, const char *query, int tolerance, char **results, int *count) {
    if (!root) return;

    int dist = levenshtein_distance(root->word, query);

    if (dist <= tolerance) {
        results[*count] = strdup_custom(root->word);
        (*count)++;
    }

    BKTreeNode *child = root->children;
    while (child) {
        if (child->distance >= dist - tolerance && child->distance <= dist + tolerance) {
            get_similar_words(child, query, tolerance, results, count);
        }
        child = child->next;
    }
}

void free_bktree(BKTreeNode *root) {
    if (!root) return;
    free_bktree(root->children);
    free_bktree(root->next);
    free(root->word);
    free(root);
}
