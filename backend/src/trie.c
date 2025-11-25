#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "trie.h"

TrieNode *create_node() {
    TrieNode *node = (TrieNode *)malloc(sizeof(TrieNode));
    node->is_end_of_word = false;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        node->children[i] = NULL;
    }
    return node;
}

void insert_trie(TrieNode *root, const char *key) {
    TrieNode *crawl = root;
    for (int i = 0; i < strlen(key); i++) {
        int index = tolower(key[i]) - 'a';
        if (index < 0 || index >= ALPHABET_SIZE) continue; // Only support a-z for now
        
        if (!crawl->children[index]) {
            crawl->children[index] = create_node();
        }
        crawl = crawl->children[index];
    }
    crawl->is_end_of_word = true;
}

bool search_trie(TrieNode *root, const char *key) {
    TrieNode *crawl = root;
    for (int i = 0; i < strlen(key); i++) {
        int index = tolower(key[i]) - 'a';
        if (index < 0 || index >= ALPHABET_SIZE) return false;
        
        if (!crawl->children[index]) {
            return false;
        }
        crawl = crawl->children[index];
    }
    return (crawl != NULL && crawl->is_end_of_word);
}

void collect_words(TrieNode *node, char *prefix, char **results, int *count) {
    if (node->is_end_of_word) {
        results[*count] = strdup(prefix);
        (*count)++;
    }
    
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) {
            char new_prefix[100];
            sprintf(new_prefix, "%s%c", prefix, i + 'a');
            collect_words(node->children[i], new_prefix, results, count);
        }
    }
}

void get_suggestions(TrieNode *root, const char *prefix, char **results, int *count) {
    TrieNode *crawl = root;
    *count = 0;
    
    for (int i = 0; i < strlen(prefix); i++) {
        int index = tolower(prefix[i]) - 'a';
        if (index < 0 || index >= ALPHABET_SIZE) return;
        
        if (!crawl->children[index]) {
            return;
        }
        crawl = crawl->children[index];
    }
    
    char buffer[100];
    strcpy(buffer, prefix);
    collect_words(crawl, buffer, results, count);
}
