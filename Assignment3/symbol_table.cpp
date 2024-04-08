#include "symbol_table.h"

/* Define the symbol table array */
symbol* SYMTAB[SIZE];

/* Hash function to compute bucket index */
int hash_function(char* specifier) {
    int len = strlen(specifier);
    int hash = 0;
    for (int i = 0; i < len; i++) 
        hash += (int)specifier[i];
    return hash % SIZE;
}

/* Initialize symbol table */
void init_symbol_table() {
    for (int i = 0; i < SIZE; i++) 
        SYMTAB[i] = nullptr;
}

/* Search for a symbol in the symbol table */
symbol* search_symbol(char* specifier) {
    int i = hash_function(specifier);
    symbol* temp = SYMTAB[i];
    while (temp != nullptr && strcmp(temp->specifier, specifier))
        temp = temp->next;
    return temp;
}

/* Insert a new symbol into the symbol table */
symbol* insert_symbol(char* specifier, char type) {
    int i = hash_function(specifier);
    symbol* new_symbol = (symbol*)malloc(sizeof(symbol));
    new_symbol->specifier = (char*)strdup(specifier);
    new_symbol->type = type;
    new_symbol->next = SYMTAB[i];
    SYMTAB[i] = new_symbol;
    return new_symbol;
}

/* Print the contents of the symbol table */
void print_symbol_table() {
    cout << "-------------------------------------\n";
    cout << "SYMTAB\n";
    for (int i = 0; i < SIZE; i++) {
        cout << "bucket [" << i << "]: ";
        symbol* temp = SYMTAB[i];
        while (temp != nullptr) {
            cout << temp->type << temp->specifier << " -> ";
            temp = temp->next;
        }
        cout << "nullptr\n";
    }
    cout << "-------------------------------------\n";
}
