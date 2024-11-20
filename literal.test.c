//
// Created by Matt on 11/13/2024.
//

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "literal_table.h"
#include "utilities.h"
#include "machine_types.h"

// Constant table entries

static literal_table_entry_t *first = NULL;
static literal_table_entry_t *last = NULL;
static unsigned int next_word_offset = 0;

// Iteration state follows
static bool iterating = false;
static literal_table_entry_t *iteration_next = NULL;
// The table of constants is a linked list of literal_table_entry_t's
// with 'first' pointing to the first entry and 'last' to the last one


// Check the invariant
static void literal_table_okay()
{
    bool emp = literal_table_empty();
    assert(emp == (next_word_offset == 0));
    assert(emp == (first == NULL));
    assert(emp == (last == NULL));
}

// Return true if the literal table is empty
bool literal_table_empty()
{
    return next_word_offset == 0;
}

// Return the size (in words/entries) in the literal table
unsigned int literal_table_size()
{
    return next_word_offset;
}

// Is the literal_table full?
bool literal_table_full()
{
    return false;
}
bool initalized = false;

// Initialize the literal_table
void literal_table_initialize()
{
    initalized=true;
    // Free any existing entries
    literal_table_entry_t *entry = first;
    while (entry != NULL) {
        literal_table_entry_t *next = entry->next;
        // If using strdup, uncomment the next line
        // free((char *)entry->text);
        free(entry);
        entry = next;
    }

    first = NULL;
    last = NULL;
    next_word_offset = 0;
    iterating = false;
    iteration_next = NULL;
    literal_table_okay();
}

// Return the offset of 'sought' if it is in the table; otherwise return -1.
int literal_table_find_offset(const char *sought, word_type value)
{
    literal_table_okay();
    literal_table_entry_t *entry = first;
    while (entry != NULL) {
        if (strcmp(entry->text, sought) == 0) {
            return entry->offset;
        }
        entry = entry->next;
    }
    return -1;
}

// Return true if 'sought' is in the table
bool literal_table_present(const char *sought, word_type value)
{
    literal_table_okay();
    return literal_table_find_offset(sought, value) >= 0;
}

// Return the word offset for val_string/value, entering it in the table if it's not already present
unsigned int literal_table_lookup(const char *val_string, word_type value)
{
    if (!initalized) {
        literal_table_initialize();

    }
    int ret = literal_table_find_offset(val_string, value);
    if (ret >= 0) {
        return ret;  // Value already exists
    }

    // Allocate a new entry
    literal_table_entry_t *new_entry = (literal_table_entry_t *)malloc(sizeof(literal_table_entry_t));
    if (!new_entry) {
        bail_with_error("Failed to allocate memory for literal table entry.");
    }

    // Assign the string directly
    new_entry->text = val_string;
    new_entry->value = value;
    new_entry->offset = next_word_offset++;
    new_entry->next = NULL;

    // Add to the table
    if (last) {
        last->next = new_entry;
    } else {
        first = new_entry;
    }
    last = new_entry;

    literal_table_okay();
    return new_entry->offset;
}

// === Iteration helpers ===

// Start an iteration over the literal table
void literal_table_start_iteration()
{
    if (iterating) {
        bail_with_error("Attempt to start literal_table iterating when already iterating!");
    }
    literal_table_okay();
    iterating = true;
    iteration_next = first;
}

// Is there another literal in the literal table?
bool literal_table_iteration_has_next()
{
    literal_table_okay();
    bool ret = (iteration_next != NULL);
    if (!ret) {
        iterating = false;
    }
    return ret;
}

// Return the next literal value in the literal table and advance the iteration
word_type literal_table_iteration_next()
{
    assert(iteration_next != NULL);

    word_type ret = iteration_next->value;
    iteration_next = iteration_next->next;
    return ret;
}

// End the current iteration over the literal table.
void literal_table_end_iteration()
{
    iterating = false;
}

// Function to print the current state of the literal table for debugging
void literal_table_debug_print()
{
    printf("Debug: Literal Table State:\n");
    literal_table_entry_t *entry = first;
    while (entry != NULL) {
        printf("Offset: %u, Text: %s, Value: %d\n", entry->offset, entry->text, entry->value);
        entry = entry->next;
    }
}

// Test function
void literal_table_test()
{
    literal_table_initialize();

    // Adding literals
    literal_table_lookup("CONST_ONE", 1);
    literal_table_lookup("CONST_TWO", 20);
    literal_table_lookup("CONST_THREE", 3);

    // Printing the table for debugging
    literal_table_debug_print();

    // Iterating over literals
    literal_table_start_iteration();
    while (literal_table_iteration_has_next()) {
        word_type value = literal_table_iteration_next();
        printf("Literal value: %u\n", value);
    }
    literal_table_end_iteration();

    // Clean up
    literal_table_initialize();  // Frees the table
}
