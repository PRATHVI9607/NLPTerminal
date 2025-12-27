/**
 * Suggestion Engine Header - Real-time command suggestions
 * Provides intellisense-like suggestions as user types
 */

#ifndef SUGGESTION_ENGINE_H
#define SUGGESTION_ENGINE_H

#include "nlp_engine.h"

#define MAX_HISTORY_SUGGESTIONS 100
#define MAX_CMD_SUGGESTIONS 50

// Command info structure
typedef struct {
    char name[64];
    char description[256];
    char usage[256];
    char examples[3][128];
} CommandInfo;

// Initialize suggestion engine with command list
void suggestion_init(void);

// Get suggestions for partial command (prefix matching)
void suggestion_get_commands(const char *prefix, SuggestionList *out);

// Get suggestions based on command context (e.g., after "cd" suggest directories)
void suggestion_get_contextual(const char *cmd, const char *partial_arg, SuggestionList *out);

// Get file/directory suggestions for path completion
void suggestion_get_paths(const char *partial_path, int dirs_only, SuggestionList *out);

// Add command to history for better suggestions
void suggestion_add_to_history(const char *cmd);

// Get suggestions from command history
void suggestion_get_from_history(const char *prefix, SuggestionList *out);

// Get command info
CommandInfo* suggestion_get_command_info(const char *cmd);

// Fuzzy match score (0-100)
int suggestion_fuzzy_score(const char *str, const char *pattern);

#endif
