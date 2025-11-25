#ifndef MACROS_H
#define MACROS_H

typedef struct MacroStep {
    char *command;
    struct MacroStep *next;
} MacroStep;

typedef struct Macro {
    char *name;
    MacroStep *head;
    MacroStep *tail;
    struct Macro *next;
} Macro;

void init_macros();
void start_recording_macro(const char *name);
void add_macro_step(const char *command);
void end_recording_macro();
int run_macro(const char *name);
Macro *find_macro(const char *name);
void free_macros();

#endif
