Data Structures Report for NLPTerminal
Generated: 2025-11-25

Overview
--------
This project implements a small custom shell (`mysh`) in C with a Python Tkinter frontend. The C backend uses several in-memory data structures to provide features such as command history, prefix completion, fuzzy correction, macros, and an undo stack. This report documents each data structure, where it is defined, how it is used, time/space characteristics, ownership and memory notes, and references to the functions that operate on it.

1) History
-------------
- Definition: `History` is defined in `backend/include/history.h`.
  - Linked-list nodes: `HistoryNode { char *command; HistoryNode *next; HistoryNode *prev; }`.
  - Container: `History { HistoryNode *head; HistoryNode *tail; int count; int capacity; char **array; }`.

- Files: `backend/include/history.h`, implementation in `backend/src/history.c`.

- Purpose & Usage:
  - Holds ordered list of executed commands in two representations simultaneously:
    1. Doubly-linked list (head/tail) for sequential traversal and freeing.
    2. Dynamic array (`char **array`) for O(1) index-based access (used by get_history_at and printing by index).
  - `main.c` creates it with `init_history(10)`, then every executed command is recorded by `add_history(history, cmd)` before execution.
  - `history` builtin calls `print_history(history)` to show the captured list.

- Key functions:
  - `History *init_history(int capacity)` — allocates container, initializes linked list and array.
  - `void add_history(History *history, const char *command)` — appends a node to linked list and stores pointer in array; resizes array when capacity reached (realloc doubling strategy).
  - `const char *get_history_at(History *history, int index)` — O(1) via array access.
  - `void print_history(History *history)` — iterates array and prints commands.
  - `void free_history(History *history)` — walks linked list freeing each node->command and node; frees array and container.

- Complexity:
  - add_history: amortized O(1) (realloc occasionally O(n)).
  - get_history_at: O(1).
  - print_history: O(n) where n=count.

- Memory / Ownership:
  - `node->command` uses `strdup_custom` (malloc) and is freed in `free_history`.
  - `history->array` stores pointers to the same strings (no extra duplication) so freeing must only free the underlying strings once (done by linked-list free).

2) Trie (prefix completion)
-----------------------------
- Definition: `TrieNode` in `backend/include/trie.h`.
  - `TrieNode { TrieNode *children[ALPHABET_SIZE]; bool is_end_of_word; }` with `ALPHABET_SIZE=26` (only a-z supported).

- Files: `backend/include/trie.h`, `backend/src/trie.c`.

- Purpose & Usage:
  - Stores the set of known commands/words for fast prefix lookup and suggestions.
  - `main.c` creates a root using `create_node()` then calls `insert_trie(trie, commands[i])` for each built-in command at startup.
  - `complete <prefix>` uses `get_suggestions(trie, prefix, suggestions, &count)` which finds the node at the prefix and recursively collects words under it.

- Key functions:
  - `TrieNode *create_node()` — allocates a node with NULL children.
  - `void insert_trie(TrieNode *root, const char *key)` — inserts word by walking/creating children.
  - `bool search_trie(TrieNode *root, const char *key)` — checks membership.
  - `void get_suggestions(TrieNode *root, const char *prefix, char **results, int *count)` — fills results with strdup'd strings.

- Complexity:
  - insert/search: O(L) where L = length of word.
  - get_suggestions: O(P + R * Lavg) where P = length of prefix, R = number of results, Lavg = average suffix length; traversal collects words recursively.

- Memory / Ownership:
  - Nodes allocated via `malloc`; results returned by `get_suggestions` are `strdup`ed and must be freed by the caller (main frees each suggestion after printing).
  - Only supports lowercase a-z (non-letters are ignored by insert/search implementation).

3) BK-tree (fuzzy matching)
-----------------------------
- Definition: `BKTreeNode` in `backend/include/bktree.h`.
  - Simplified structure: `BKTreeNode { char *word; BKTreeNode *children; BKTreeNode *next; int distance; }`.
  - The `children` pointer is used as a singly-linked list of child nodes; `next` is sibling pointer.

- Files: `backend/include/bktree.h`, `backend/src/bktree.c`.

- Purpose & Usage:
  - Provides fuzzy-word matching (Levenshtein distance) to suggest corrections for mistyped commands (used for `correct <word>` and as suggestions when `execvp` fails).
  - `main.c` populates the BK-tree with the known builtins via `insert_bktree(&bktree, commands[i])`.
  - When a command fails, code calls `get_similar_words(bktree, args[0], 2, suggestions, &count)` to collect suggestions within tolerance 2.

- Key functions:
  - `BKTreeNode *create_bk_node(const char *word)` — allocates a node with `strdup_custom`.
  - `void insert_bktree(BKTreeNode **root, const char *word)` — inserts a word by computing distance to node and placing in child's list by distance.
  - `int levenshtein_distance(const char *s1, const char *s2)` — full dynamic programming implementation (matrix size len1*len2) used by insert and query.
  - `void get_similar_words(BKTreeNode *root, const char *query, int tolerance, char **results, int *count)` — recursively follows children whose edge distances fall in range [dist - tol, dist + tol].
  - `void free_bktree(BKTreeNode *root)` — recursive free of children and siblings.

- Complexity:
  - levenshtein_distance: O(len1 * len2) time and O(len1*len2) stack/local memory (matrix on stack in current implementation).
  - insert/query complexity depends on tree shape; average query visits a subset of nodes based on tolerance; worst-case O(N * L^2) with N words.

- Memory / Ownership:
  - `word` strings duplicated with `strdup_custom` and freed by `free_bktree`.
  - Note: `levenshtein_distance` uses a VLA matrix on stack sized by lengths; very long strings could overflow stack — consider heap allocation for large inputs.

4) Macros
-----------
- Definition: In `backend/include/macros.h`:
  - `MacroStep { char *command; MacroStep *next; }` (singly-linked list of steps)
  - `Macro { char *name; MacroStep *head; MacroStep *tail; Macro *next; }` (linked list of macros)

- Files: `backend/include/macros.h`, `backend/src/macros.c`.

- Purpose & Usage:
  - Records sequences of commands while in recording mode (`macro define <name>` ... `macro end`) and stores them in a global linked list `macro_list`.
  - `main.c` uses `start_recording_macro`, `add_macro_step` and `end_recording_macro` while taking commands; when `macro run <name>` is invoked `main.c` retrieves the macro (`find_macro`) and iterates its `MacroStep` list to call `execute_line` for each step.

- Key functions:
  - `start_recording_macro(const char *name)`, `add_macro_step(const char *command)`, `end_recording_macro()`, `Macro *find_macro(const char *name)`, `free_macros()`.

- Complexity & Memory:
  - Recording a macro appends steps in O(1) per step (using tail pointer).
  - Macro lookup `find_macro` is O(M) where M = number of macros (linear scan of linked list).
  - Steps/commands duplicated via `strdup_custom` — freed by `free_macros`.

5) Undo stack
--------------
- Definition: `UndoStack` in `backend/include/undo.h`.
  - `UndoNode { char *command; UndoNode *next; }` and `UndoStack { UndoNode *top; int count; }`.

- Files: `backend/include/undo.h`, `backend/src/undo.c`.

- Purpose & Usage:
  - Stores commands (or command strings representing reversible operations) as a stack: push before potentially reversible operations and pop when `undo` is invoked.
  - `main.c` calls `push_undo(undo_stack, cmd)` after successful operations in many places (for example after `cd` or after a successful external command) and handles `undo` by calling `pop_undo`.

- Key functions:
  - `UndoStack *init_undo_stack()` — allocate stack.
  - `void push_undo(UndoStack *stack, const char *command)` — push a strdup'd command.
  - `char *pop_undo(UndoStack *stack)` — pop and return command pointer (caller frees it after use in main).
  - `void free_undo_stack(UndoStack *stack)` — free all nodes and strings.

- Complexity:
  - push/pop are O(1).

6) Utility helpers
-------------------
- Files: `backend/include/utils.h`, `backend/src/utils.c`.
- Functions:
  - `char *strdup_custom(const char *s)` — simple wrapper that mallocs and copies a string.
  - `void trim_whitespace(char *str)` — trims leading/trailing whitespace in-place.

- Purpose & Usage:
  - Used extensively to allocate copies of input strings that are stored in the data structures (history, macros, BK-tree nodes, undo entries).

Global state and orchestration
-------------------------------
- `main.c` creates and wires these data structures at startup:
  - `History *history = init_history(10);`
  - `TrieNode *trie = create_node();`
  - `BKTreeNode *bktree = NULL;` then populated by `insert_bktree` for each known command.
  - `UndoStack *undo_stack = init_undo_stack();`
  - `init_macros();` to set up macro globals.

- The program populates `trie` and `bktree` with a static list of known commands early in `main`.
- The interactive loop records each input into `history` via `add_history` and then calls `execute_line(...)` which uses these structures for builtins, completion, correction and macros.

Memory-management notes and safety
----------------------------------
- Most structures allocate strings with `strdup_custom` and free them in corresponding `free_*` functions. That convention is consistent across history, bktree, macros, and undo.
- The trie stores no duplicated strings — it stores structure nodes only. `get_suggestions` duplicates returned suggestion strings with `strdup` and expects caller to free them.
- The BK-tree `levenshtein_distance` function allocates a full matrix on the stack (VLA). This is simple/fast for short command strings but can risk stack overflow for large strings; consider switching to a single row DP (O(min(len1,len2)) memory) or heap allocation.
- No explicit thread-safety: the code assumes single-threaded execution (the frontend uses subprocess communication and the shell runs in parent process). If later multi-threaded access is added, explicit synchronization is required.

Potential improvements and recommendations
----------------------------------------
- History:
  - Current design stores strings once and shares pointers between list and array — this is memory efficient but coupling requires correct free ordering which is implemented now.
  - Add a bounded circular buffer option to avoid unbounded memory growth in long sessions.

- Trie:
  - Extend alphabet or use a map/dictionary for non-letter characters (hyphens, underscores, digits).
  - Add a free_trie function to recursively free nodes (currently commented out in `main.c`).

- BK-tree:
  - Avoid VLA matrix; switch to Myers or iterative DP with O(min(m,n)) memory.
  - Consider limiting tolerance dynamically by word length.

- Macros:
  - `run_macro` currently returns an int; `main.c` handles actual step execution. Consider exposing an iterator or an API to execute macro steps directly from `macros.c` to keep logic encapsulated.

- Undo stack:
  - Currently stores command strings; to make undo actionable for filesystem operations, store structured reverse operations (e.g., for `rm` store file contents or mark operations that are non-reversible).

References (files inspected)
---------------------------
- backend/include/history.h
- backend/src/history.c
- backend/include/trie.h
- backend/src/trie.c
- backend/include/bktree.h
- backend/src/bktree.c
- backend/include/macros.h
- backend/src/macros.c
- backend/include/undo.h
- backend/src/undo.c
- backend/include/utils.h
- backend/src/utils.c
- backend/include/commands.h
- backend/src/commands.c
- backend/src/main.c

End of report.
