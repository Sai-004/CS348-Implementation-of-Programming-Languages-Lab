#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

/* header files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Constants */
#define NOT_INITIALIZED -10000000

/* declaration of useful functions and variables */

/* Function prototypes */
char *install_id();            // Install an identifier
int install_int();             // Install an integer
float install_real();          // Install a real number

/* Global variable for line number */
int line = 0;

/* Linked list implementation */
typedef struct node
{
    char *specifier;           // Identifier specifier
    struct node *next;         // Pointer to the next node
} node;

node *head = NULL;             // Head pointer for the linked list

/* Hashtable implementation */
#define SIZE 20                // Size of the hashtable array

typedef struct symbol
{
    char *specifier;           // Identifier specifier
    float real;                // Real number value
    int integer;               // Integer value
    char type;                 // Type of the identifier ('I' for integer, 'F' for float)
    int initialized;           // Flag indicating whether the identifier is initialized
    int declared;              // Flag indicating whether the identifier is declared
    struct symbol *next;       // Pointer to the next symbol in the hashtable bucket
} symbol;

symbol *SYMTAB[SIZE];          // Hashtable array

// Hash function to calculate bucket index
int hash_function(char *specifier)
{
    int len = strlen(specifier);
    // simple hash function
    int hash = 0;
    for (int i = 0; i < len; i++)
        hash += (int)specifier[i];
    return hash % SIZE;
}

/* Function prototypes */                
int search_id(char *specifier);                      // Search for an identifier in the linked list
int add_id(char *specifier);                         // Add an identifier to the linked list
void print_id_list();                                // Print the linked list
void init();                                         // Initialize the symbol table
struct symbol *search(char *specifier);              // Search for a symbol in the symbol table
struct symbol *insert(char *specifier, char type);   // Insert a symbol into the symbol table
void print_hash_table();                             // Print the symbol table
int get_line_number();                               // Get the current line number
int get_int_value(char *specifier);                  // Get the integer value of a symbol
float get_real_value(char *specifier);               // Get the real value of a symbol
void set_int_value(char *specifier, int val);        // Set the integer value of a symbol
void set_real_value(char *specifier, float val);     // Set the real value of a symbol
int is_declared(char *specifier);                    // Check if a symbol is declared
int is_initialized(char *specifier);                 // Check if a symbol is initialized
int get_type(char *specifier);                       // Get the type of a symbol
int set_type(int type);                              // Set the type of symbols in the declaration list
int set_init();                                      // Initialize variables in the id list while reading
void clear_list();                                   // Clear the linked list

#endif /* SYMBOL_TABLE_H */
