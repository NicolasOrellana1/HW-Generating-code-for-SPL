// $Id: literal_table.h,v 1.2 2023/11/13 12:45:52 leavens Exp $
#ifndef _LITERAL_TABLE_H
#define _LITERAL_TABLE_H
#include <stdbool.h>
#include "machine_types.h"

// Initializes the literal table, freeing any existing entries.
void literal_table_initialize();

// Returns true if the literal table is empty.
bool literal_table_empty();

// Returns the size of the literal table (number of entries).
unsigned int literal_table_size();

// Adds a literal value to the table or retrieves its existing offset.
// Returns the offset of the literal.
unsigned int literal_table_lookup(int value);

// Starts an iteration over the literal table.
void literal_table_start_iteration();

// Checks if there are more literals in the current iteration.
bool literal_table_iteration_has_next();

// Returns the next literal during iteration.
int literal_table_iteration_next();

// Ends the current iteration.
void literal_table_end_iteration();

// Prints the state of the literal table for debugging.
void literal_table_debug_print();

#endif // LITERAL_TABLE_H