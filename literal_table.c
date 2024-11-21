#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "literal_table.h"

// Structure representing an entry in the literal table
typedef struct literal_table_entry {
    struct literal_table_entry *next; // Pointer to the next entry
    int value;                       // The integer value of the literal
    unsigned int offset;             // Offset assigned to the literal
} literal_table_entry_t;

// Global variables
static literal_table_entry_t *first = NULL;   // First entry in the table
static literal_table_entry_t *last = NULL;    // Last entry in the table
static unsigned int next_offset = 0;          // Next available offset
static bool iterating = false;                // Iteration state
static literal_table_entry_t *current_iter = NULL;

// === Utility Functions ===

// Initialize the literal table
void literal_table_initialize() {
    // Free any existing entries
    literal_table_entry_t *entry = first;
    while (entry) {
        literal_table_entry_t *next = entry->next;
        free(entry);
        entry = next;
    }
    first = NULL;
    last = NULL;
    next_offset = 0;
    iterating = false;
    current_iter = NULL;
}

// Check if the literal table is empty
bool literal_table_empty() {
    return next_offset == 0;
}

// Return the size of the literal table (number of entries)
unsigned int literal_table_size() {
    return next_offset;
}

// === Core Functions ===

// Lookup or add a literal to the table
unsigned int literal_table_lookup(int value) {
    // Search for an existing entry
    literal_table_entry_t *entry = first;
    while (entry) {
        if (entry->value == value) {
            return entry->offset; // Found, return the existing offset
        }
        entry = entry->next;
    }

    // Not found, add a new entry
    literal_table_entry_t *new_entry = malloc(sizeof(literal_table_entry_t));
    if (!new_entry) {
        fprintf(stderr, "Error: Unable to allocate memory for literal table entry.\n");
        exit(EXIT_FAILURE);
    }
    new_entry->value = value;
    new_entry->offset = next_offset++;
    new_entry->next = NULL;

    if (last) {
        last->next = new_entry;
    } else {
        first = new_entry;
    }
    last = new_entry;

    return new_entry->offset;
}

// === Iteration Functions ===

// Start iterating over the literal table
void literal_table_start_iteration() {
    if (iterating) {
        fprintf(stderr, "Error: Already iterating over literal table.\n");
        exit(EXIT_FAILURE);
    }
    iterating = true;
    current_iter = first;
}

// Check if there are more literals to iterate
bool literal_table_iteration_has_next() {
    return current_iter != NULL;
}

// Get the next literal during iteration
int literal_table_iteration_next() {
    if (!current_iter) {
        fprintf(stderr, "Error: No more literals to iterate.\n");
        exit(EXIT_FAILURE);
    }
    int value = current_iter->value;
    current_iter = current_iter->next;
    return value;
}

// End the current iteration
void literal_table_end_iteration() {
    iterating = false;
    current_iter = NULL;
}

// Debug function to print the literal table
void literal_table_debug_print() {
    printf("Literal Table:\n");
    printf("Offset | Value\n");
    printf("-------+-------\n");
    literal_table_entry_t *entry = first;
    while (entry) {
        printf("%6u | %d\n", entry->offset, entry->value);
        entry = entry->next;
    }
    printf("\n");
}
