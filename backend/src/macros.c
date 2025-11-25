#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "macros.h"
#include "utils.h"

// Global state for macros
static Macro *macro_list = NULL;
static Macro *current_recording_macro = NULL;

void init_macros() {
    macro_list = NULL;
    current_recording_macro = NULL;
}

void start_recording_macro(const char *name) {
    if (current_recording_macro) {
        printf("Error: Already recording macro '%s'.\n", current_recording_macro->name);
        return;
    }
    
    // Check if macro exists
    if (find_macro(name)) {
        printf("Error: Macro '%s' already exists.\n", name);
        return;
    }

    Macro *new_macro = malloc(sizeof(Macro));
    new_macro->name = strdup_custom(name);
    new_macro->head = NULL;
    new_macro->tail = NULL;
    new_macro->next = NULL;

    current_recording_macro = new_macro;
    printf("Started recording macro '%s'. Type 'macro end' to stop.\n", name);
}

void add_macro_step(const char *command) {
    if (!current_recording_macro) return;
    
    // Don't record the 'macro end' command itself, handled in main
    MacroStep *step = malloc(sizeof(MacroStep));
    step->command = strdup_custom(command);
    step->next = NULL;

    if (current_recording_macro->tail) {
        current_recording_macro->tail->next = step;
    } else {
        current_recording_macro->head = step;
    }
    current_recording_macro->tail = step;
}

void end_recording_macro() {
    if (!current_recording_macro) {
        printf("Error: Not recording any macro.\n");
        return;
    }

    // Add to list
    current_recording_macro->next = macro_list;
    macro_list = current_recording_macro;
    
    printf("Macro '%s' saved.\n", current_recording_macro->name);
    current_recording_macro = NULL;
}

Macro *find_macro(const char *name) {
    Macro *curr = macro_list;
    while (curr) {
        if (strcmp(curr->name, name) == 0) return curr;
        curr = curr->next;
    }
    return NULL;
}

// We need a callback or way to execute commands. 
// Since we can't easily call main's logic, we might need to expose a 'execute_command_string' function in main, 
// or return the steps to main.
// Simpler: run_macro returns 1 if found, and main handles the execution loop if it has access to the steps.
// Or better: We pass a function pointer? No, too complex for this step.
// Let's just return 1 if found, and let main retrieve steps.
// Actually, let's just print the steps for now, or better, implement a `get_macro_steps` helper.

int run_macro(const char *name) {
    Macro *m = find_macro(name);
    if (!m) {
        printf("Error: Macro '%s' not found.\n", name);
        return 0;
    }
    
    printf("Running macro '%s'...\n", name);
    // In a real implementation, we would execute these. 
    // But here we are inside a library. 
    // We will return 1 to indicate success, and main will need to iterate.
    return 1;
}

void free_macros() {
    Macro *curr = macro_list;
    while (curr) {
        Macro *next = curr->next;
        MacroStep *step = curr->head;
        while (step) {
            MacroStep *next_step = step->next;
            free(step->command);
            free(step);
            step = next_step;
        }
        free(curr->name);
        free(curr);
        curr = next;
    }
}
