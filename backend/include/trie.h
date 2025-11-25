#ifndef TRIE_H
#define TRIE_H

#include <stdbool.h>

#define ALPHABET_SIZE 26

typedef struct TrieNode {
    struct TrieNode *children[ALPHABET_SIZE];
    bool is_end_of_word;
} TrieNode;

TrieNode *create_node();
void insert_trie(TrieNode *root, const char *key);
bool search_trie(TrieNode *root, const char *key);
void get_suggestions(TrieNode *root, const char *prefix, char **results, int *count);

#endif
