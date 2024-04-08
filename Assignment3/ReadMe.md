# Lexical Analyzer

## Introduction

This project implements a Lexical Analyzer for a simple programming language. The language includes keywords such as PROGRAM, VAR, BEGIN, END, INTEGER, FOR, READ, WRITE, TO, DO, along with symbols such as ;, :, ,, :=, +, -, *, DIV, (, and ). Additionally, it handles identifiers (IDs) and integers.

## Files Included

1. `lexicalAnalyser.l`: Contains the Lex code defining patterns, manifest constants, token recognition rules, and the main function for lexing and parsing.

2. `symbol_table.h`: Header file with the structure definition for symbols and function prototypes for symbol table operations.

3. `symbol_table.cpp`: Implementation file with functions for initializing the symbol table, searching for symbols, inserting symbols, and printing the symbol table.

## Usage

### Environment Setup

- **Operating System:** *Ubuntu 22*

- **Install Flex and Bison:**
    ```
    sudo apt-get install bison flex
    ```

### Compilation

- **Compile the Lex code:**
    ```
    flex lexicalAnalyser.l
    ```

- **Compile the C++ code:**
    ```
    g++ lex.yy.c symbol_table.cpp -o lexer_program
    ```

### Execution

- **Run the compiled executable:**
    ```
    ./lexer_program < input.pas
    ```

## Output

- The program generates an output file named `output.txt` containing tokenized symbols along with their types and specifiers.
- The symbol table is printed to the console, showing identifiers and numbers along with their types.

## Symbol Table

- The symbol table is implemented as an array of linked lists, with symbols hashed based on their specifiers (identifiers or numbers).
- Functions are provided for initializing the table, hashing identifiers, searching for symbols, inserting symbols, and printing the table.

## Example Usage

Suppose you have a program written in the supported language. Running the Lexical Analyzer on this program will tokenize it, identify keywords, symbols, identifiers, and numbers, and store them in the symbol table for further processing.

## Notes

- Ensure Flex (Lex) is installed to compile the Lex code (`lexicalAnalyser.l`).
- The lexer program detects and handles invalid tokens encountered in the input text by printing an error message to the console.
