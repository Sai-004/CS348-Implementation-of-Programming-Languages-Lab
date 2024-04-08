#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#define SIZE 20

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

/* Define the symbol structure */
typedef struct symbol {
    char* specifier;
    char type;
    struct symbol* next;
} symbol;

/* Function prototypes for symbol table operations */
void init_symbol_table();
symbol* search_symbol(char* specifier);
symbol* insert_symbol(char* specifier, char type);
void print_symbol_table();

#endif /* SYMBOL_TABLE_H */
