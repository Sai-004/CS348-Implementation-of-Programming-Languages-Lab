# YACC Parser

## Introduction

This project comprises a YACC Parser designed to parse a simple programming language and handle semantic errors such as duplicate declaration, missing declaration, missing initialization, and type mismatch. The parser interacts with a Lexical Analyzer to tokenize the input code.

## Files Included

1. `parser.y`: Contains the YACC code defining grammar rules, semantic actions, and error handling procedures.
2. `lexer.l`: Lexical Analyzer file used in conjunction with the parser to tokenize input code.
3. `symbol_table.h`: Header file defining structures and function prototypes for symbol table operations.
4. `symbol_table.c`: Implementation file with functions for initializing the symbol table, hashing, searching, inserting symbols, and printing the symbol table.
5. `run.sh`: Bash script to compile and execute the parser with sample input files.

## Usage

### Environment Setup

- **Operating System:** Ubuntu 22

- **Installation:**
    ```
    sudo apt-get install bison flex
    ```

### Compilation

- **Ensure script has executable permissions:**
    ```
    chmod +x run.sh
    ```

- **Execute the script:**
    ```
    ./run.sh
    ```

### Execution

- **Execute the compiled parser with input files:**
    ```
    ./a.out < input.pas
    ./a.out < input_error.pas
    ```

## Semantic Errors Handled

1. Duplicate declaration
2. Missing declaration
3. Missing initialization
4. Type Mismatch

## Output

The parser will detect and handle semantic errors in the input code, such as duplicate declarations, missing declarations, missing initializations, and type mismatches. Additionally, it will print the symbol table after error detection.

## Symbol Table

The symbol table is implemented using a hash table with separate chaining. Symbols are hashed based on their identifiers, and each entry in the table contains information such as the specifier string, type, integer or float value, and flags indicating declaration and initialization status. Functions are provided for initializing the table, hashing identifiers, searching for symbols, inserting symbols, and printing the table.

## Example Usage

Suppose you have a program written in the supported language. Running the YACC Parser on this program will parse it, detect semantic errors, and print the symbol table with relevant information.

## Assumptions (after error detections)

1. If there is a duplicate declaration, then the parser goes with the first declaration.
2. If the variable is not declared, it is not declared while assigning it values.
3. If there is a type mismatch while assigning, then the variable is not initialized.
4. If there is a mismatch in expression types, then after error, the whole expression is given type real.
5. If a variable is not declared, its expression is given type integer.