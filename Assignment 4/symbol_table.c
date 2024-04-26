/* header files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h" // Include the symbol table header file for declarations

// Function to search for an identifier in the linked list
int search_id(char *specifier)
{
    // Search the specifier in the linked list
    node *temp = head;
    while (temp != NULL && strcmp(specifier, temp->specifier))
    {
        temp = temp->next;
    }
    // Return 0 if not found, 1 if found
    return (temp != NULL);
}

// Function to add a new identifier to the beginning of the list
int add_id(char *specifier)
{
    // Allocate memory for a new node
    node *temp = (node *)malloc(sizeof(node));
    temp->specifier = (char *)strdup(specifier);
    temp->next = NULL;

    // If the id is already present, return failure
    if (search_id(specifier))
    {
        return 0;
    }

    // Add the new node to the beginning of the list
    if (head)
    {
        temp->next = head;
        head = temp;
    }
    else
        head = temp;

    // Return 0 for failure, 1 for success
    return 1;
}

// Function to print the linked list
void print_id_list()
{
    node *temp = head;
    while (temp != NULL)
    {
        printf("%s -> ", temp->specifier);
        temp = temp->next;
    }
    printf("NULL\n");
}

// Initialize all buckets of the hash table to null
void init()
{
    for (int i = 0; i < SIZE; i++)
        SYMTAB[i] = NULL;
}

// Search for a symbol in the symbol table using hashing
symbol *search(char *specifier)
{
    int i = hash_function(specifier);

    // Search the linked list associated with the bucket
    symbol *temp = SYMTAB[i];
    while (temp != NULL && strcmp(temp->specifier, specifier))
        temp = temp->next;

    return temp;
}

// Insert a symbol into the symbol table using hashing
symbol *insert(char *specifier, char type)
{
    int i = hash_function(specifier);

    // Create a new symbol
    symbol *new_symbol = (symbol *)malloc(sizeof(symbol));
    new_symbol->specifier = (char *)strdup(specifier);
    new_symbol->real = NOT_INITIALIZED;
    new_symbol->integer = NOT_INITIALIZED;
    new_symbol->type = type;
    new_symbol->initialized = 0;
    new_symbol->declared = 0;

    // Insert it at the beginning of the bucket linked list
    new_symbol->next = SYMTAB[i];
    SYMTAB[i] = new_symbol;

    return new_symbol;
}

// Print the hash table
void print_hash_table()
{
    printf("-------------------------------------------------------------------------------------\n");
    printf("SYMTAB\n");
    for (int i = 0; i < SIZE; i++)
    {
        printf("bucket [%d]: ", i);
        symbol *temp = SYMTAB[i];
        while (temp != NULL)
        {
            printf("<%s %c : ", temp->specifier, temp->type);
            printf("{ ");
            printf("declared: %d, initialized: %d ", temp->declared, temp->initialized);
            printf("}> -> ");
            temp = temp->next;
        }
        printf("NULL\n");
    }
    printf("-------------------------------------------------------------------------------------\n");
}

// Get the current line number
int get_line_number()
{
    return line;
}

// Get the integer value of a symbol
int get_int_value(char *specifier)
{
    symbol *sym = search(specifier);
    if (sym != NULL)
        return sym->integer;
}

// Get the real value of a symbol
float get_real_value(char *specifier)
{
    symbol *sym = search(specifier);
    if (sym != NULL)
        return sym->real;
}

// Set the integer value of a symbol
void set_int_value(char *specifier, int val)
{
    symbol *sym = search(specifier);
    if (sym != NULL)
    {
        sym->integer = val;
        sym->type = 'I';
        sym->initialized = 1;
    }
}

// Set the real value of a symbol
void set_real_value(char *specifier, float val)
{
    symbol *sym = search(specifier);
    if (sym != NULL)
    {
        sym->real = val;
        sym->type = 'F';
        sym->initialized = 1;
    }
}

// Check if a symbol is declared
int is_declared(char *specifier)
{
    symbol *sym = search(specifier);
    if (sym != NULL)
        return (sym->declared);
}

// Check if a symbol is initialized
int is_initialized(char *specifier)
{
    symbol *sym = search(specifier);
    if (sym != NULL)
    {
        return (sym->initialized);
    }
}

// Get the type of a symbol
int get_type(char *specifier)
{
    symbol *sym = search(specifier);
    if (sym != NULL)
    {
        if (sym->type == 'I')
            return 0;
        if (sym->type == 'F')
            return 1;
        if (sym->type == 'V')
            return 2;
    }
}

// Set the type of symbols in the declaration list
int set_type(int type)
{
    node *temp = head;
    int flag = 1;
    while (temp != NULL)
    {
        symbol *sym = search(temp->specifier);
        if (sym->declared)
            flag = 0; // Duplicate error
        else
        {
            if (type == 0)
                sym->type = 'I';
            if (type == 1)
                sym->type = 'F';
            sym->declared = 1;
        }
        temp = temp->next;
    }
    head = NULL;
    return flag;
}

// Initialize variables in the id list while reading
int set_init()
{
    node *temp = head;
    int flag = 1;
    while (temp != NULL)
    {
        symbol *sym = search(temp->specifier);
        if (sym->declared == 0) // Missing declaration
            flag = 0;
        else
            sym->initialized = 1;
        temp = temp->next;
    }
    head = NULL;
    return flag;
}

// Clear the linked list
void clear_list()
{
    head = NULL;
}
