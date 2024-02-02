# Two-Pass Assembler

## Overview

This project implements a two-pass assembler in C++ to process assembly language programs. The assembler consists of two main components: `PassOne.cpp` for the first pass and `PassTwo.cpp` for the second pass.

## Assumptions and Remarks

- Ensure you have a file named `input.dat` with the input assembly code in the same directory as the assembler.
- *Input Files:* The input files (`input.dat`, `opTab.dat`, and `symTab.dat`) are correctly formatted.
- The assembler in pass 2 uses two input files (`opTab.dat` and `symTab.dat`) for stored opcode and symbol information, respectively.
- *Output Files:* Three files, namely `intermediate.dat`, `listing.dat`, and `output.dat`, will be generated at the end of the execution.
- *Error Handling:* If there is any error, it will be printed on the terminal. Review the terminal output for any error messages.
- *Comments:* Comments are assumed to have a `"."` symbol at the begining.


## Commands

### 1. Compile the Programs

```bash
g++ PassOne.cpp -o pass1
g++ PassTwo.cpp -o pass2
```

### 2. Run the First Pass

```bash
./pass1
```

This will generate an intermediate file `intermediate.dat` and a symbol table file `symTab.dat`.

### 3. Run the Second Pass

```bash
./pass2
```

This will produce an output file `output.dat` and a listing file `listing.dat`.

## Input Files

- `input.dat`: Contains the assembly language source code.
- `opTab.dat`: Contains the opcode table.
- `symTab.dat`: Will be generated after the first pass, storing symbol information.

## Output Files

- `intermediate.dat`: Generated after the first pass, containing the intermediate code.
- `output.dat`: Generated after the second pass, containing the final machine code.
- `listing.dat`: Generated after the second pass, containing a detailed listing of the assembly code with machine code.

## Additional Notes

- Ensure that you run the first pass before the second pass.
- Review the generated output and listing files for checking the correctness.