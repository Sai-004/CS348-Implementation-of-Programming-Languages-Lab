# SIC/XE 2 PASS ASSEMBLER AND LINKER LOADER

## Overview

This project implements a two-pass assembler and a linker loader in C++ to process assembly language programs for the SIC/XE architecture. The assembler is divided into two main components: `assembler_pass1.cpp` and `assembler_pass2.cpp` for the assembly process and `linker_loader.cpp` for linking and loading the assembled code.

## Assumptions and Remarks

- Ensure you have a file named `input.dat` with the input assembly code in the same directory as the assembler.
- *Input Files:* The input files (`input.dat`, `opTab.dat`) are correctly formatted.
- *Output Files:* 5 major output files, namely `intermediate.dat`, `listing.dat`, `output.dat`, `memory.dat`, and `exSymTab.dat` will be generated at the end of the execution. Output files `symTab.dat` and `litTab.dat` are also generated in assembler_pass1 which are used in assembler_pass2.
- The linker loader operates based on assumptions regarding the format of the input record files and the memory layout.
- *Error Handling:* Proper error handling is implemented, and any errors encountered during assembly or linking/loading are reported on the terminal.
- *Comments:* Comments are assumed to have a `"."` symbol at the begining.

## How to Run

### Assembler:

```bash
g++ assembler_pass1.cpp -o pass1
./pass1
g++ assembler_pass2.cpp -o pass2
./pass2
```

### Linker Loader:

```bash
g++ linker_loader.cpp -o linkloader
./linkloader
```

When prompted, enter the desired program address (PROGADDR).

## Input Files

- `input.dat`: Contains the assembly language source code.
- Additional input files may be required for the linker loader process, depending on the specific implementation.

## Output Files

- `output.dat`: Contains the generated object code.
- `listing.dat`: Provides a listing for each instruction.
- `intermediate.dat`: Intermediate file with appropriate LOCCTR values for each line during assembly.
- `memory.dat`: Memory visualization for the loaded program after linking and loading.
- `exSymTab.dat`: Line-wise output of modified entries during the linking and loading process.

## Additional Notes

- Ensure that input files are properly formatted and adhere to the specified guidelines.
- Ensure that you run codes in the order: `assembler_pass1` before `assembler_pass2` and `assembler_pass2` before `linker_loader`.
- Review the generated output files and memory visualization to verify the correctness of the assembled and loaded program.
- The code is designed to be run in a standard text editor or terminal environment.