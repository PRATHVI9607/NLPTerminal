/**
 * NLP Engine Header - Natural Language Processing for Shell Commands
 * Implements pattern matching and command translation in pure C
 */

#ifndef NLP_ENGINE_H
#define NLP_ENGINE_H

#define MAX_SUGGESTIONS 10
#define MAX_SUGGESTION_LEN 256
#define MAX_PATTERN_LEN 512

// Suggestion structure
typedef struct {
    char suggestions[MAX_SUGGESTIONS][MAX_SUGGESTION_LEN];
    int count;
    int selected_index;
} SuggestionList;

// NLP Translation result
typedef struct {
    char original[MAX_PATTERN_LEN];
    char translated[MAX_PATTERN_LEN];
    int was_translated;
    char explanation[MAX_PATTERN_LEN];
} NLPResult;

// Initialize NLP engine
void nlp_init(void);

// Translate natural language to shell command
NLPResult nlp_translate(const char *input);

// Get command suggestions based on partial input (for intellisense)
void nlp_get_suggestions(const char *partial, SuggestionList *suggestions);

// Get the best suggestion for autocomplete
const char* nlp_get_best_suggestion(const char *partial);

// Check if input looks like natural language
int nlp_is_natural_language(const char *input);

// Get command description/help
const char* nlp_get_command_help(const char *cmd);

#endif
