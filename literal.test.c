#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "literal_table.h"
#include "utilities.h"
#include "machine_types.h"

// Literal table entry definition
typedef struct literal_table_entry_s {
    struct literal_table_entry_s *next;
    const char *text;
    word_type value;
    unsigned int offset;
} literal_table_entry_t;

// Stack entry definition
typedef struct stack_entry_s {
    word_type value;
    struct stack_entry_s *next;
} stack_entry_t;

// Stack definition
typedef struct stack_s {
    stack_entry_t *top;
    unsigned int size;
    unsigned int capacity;
} stack_t;

// Word register definition
typedef struct word_registers_s {
    word_type *registers;
    unsigned int size;
} word_registers_t;

// Static variables for literal table
static literal_table_entry_t *first;
static literal_table_entry_t *last;
static unsigned int next_word_offset;

// Static variables for stack
static stack_t stack;

// Static variables for word registers
static word_registers_t registers;

// Literal table utility functions
static void literal_table_okay()
{
    bool emp = (next_word_offset == 0);
    assert(emp == (first == NULL));
    assert(emp == (last == NULL));
}

// Stack utility functions
void stack_initialize(stack_t *stack, unsigned int capacity)
{
    stack->top = NULL;
    stack->size = 0;
    stack->capacity = capacity;
}

bool stack_empty(const stack_t *stack)
{
    return stack->size == 0;
}

bool stack_full(const stack_t *stack)
{
    return stack->size >= stack->capacity;
}

void stack_push(stack_t *stack, word_type value)
{
    assert(!stack_full(stack));
    stack_entry_t *new_entry = (stack_entry_t *)malloc(sizeof(stack_entry_t));
    if (!new_entry) {
        bail_with_error("No space to allocate new stack entry!");
    }
    new_entry->value = value;
    new_entry->next = stack->top;
    stack->top = new_entry;
    stack->size++;
}

word_type stack_pop(stack_t *stack)
{
    assert(!stack_empty(stack));
    stack_entry_t *top_entry = stack->top;
    word_type value = top_entry->value;
    stack->top = top_entry->next;
    free(top_entry);
    stack->size--;
    return value;
}

// Register utility functions
void registers_initialize(word_registers_t *reg, unsigned int size)
{
    reg->registers = (word_type *)malloc(size * sizeof(word_type));
    if (!reg->registers) {
        bail_with_error("No space to allocate word registers!");
    }
    reg->size = size;
    for (unsigned int i = 0; i < size; i++) {
        reg->registers[i] = 0;
    }
}

word_type registers_read(const word_registers_t *reg, unsigned int index)
{
    assert(index < reg->size);
    return reg->registers[index];
}

void registers_write(word_registers_t *reg, unsigned int index, word_type value)
{
    assert(index < reg->size);
    reg->registers[index] = value;
}

// Literal table initialization
void literal_table_initialize()
{
    first = NULL;
    last = NULL;
    next_word_offset = 0;
    literal_table_okay();
    stack_initialize(&stack, 1024); // Example stack capacity
    registers_initialize(&registers, 16); // Example register size
}

// Lookup and add in literal table
unsigned int literal_table_lookup(const char *val_string, word_type value)
{
    literal_table_okay();
    literal_table_entry_t *entry = first;
    while (entry != NULL) {
        if (strcmp(entry->text, val_string) == 0) {
            return entry->offset;
        }
        entry = entry->next;
    }

    // Add new entry
    literal_table_entry_t *new_entry = (literal_table_entry_t *)malloc(sizeof(literal_table_entry_t));
    if (!new_entry) {
        bail_with_error("No space to allocate new literal table entry!");
    }
    new_entry->text = val_string;
    new_entry->value = value;
    new_entry->next = NULL;
    new_entry->offset = next_word_offset++;

    if (!first) {
        first = new_entry;
        last = new_entry;
    } else {
        last->next = new_entry;
        last = new_entry;
    }
    return new_entry->offset;
}

// Stack and registers interaction in literal table
void literal_table_push_to_stack(const char *val_string)
{
    int offset = literal_table_lookup(val_string, 0); // Assuming a default value of 0
    stack_push(&stack, offset);
}

void literal_table_write_to_register(unsigned int index, const char *val_string)
{
    int offset = literal_table_lookup(val_string, 0); // Assuming a default value of 0
    registers_write(&registers, index, offset);
}
