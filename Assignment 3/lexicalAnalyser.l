%{
    /* Header files */
    #include "symbol_table.h" // Include the header file for symbol table implementation

    /* Declaration of useful functions and variables */
    void* yylval;
    void* install22();
    void* install_num();
    int line = 0;
%}

/* Define patterns using regular expressions */
delim       [ \t\n]
alpha       [A-Za-z]
digit       [0-9]
id          {alpha}({alpha}|{digit})*
int         {digit}+

/* Definitions of manifest constants */
%%
\n          { line++;       } // Increment line number on newline
[ \t]       {               } // Ignore spaces and tabs
PROGRAM     { return 1;     } // Return token value for PROGRAM
VAR         { return 2;     } // Return token value for VAR
BEGIN       { return 3;     } // Return token value for BEGIN
END         { return 4;     } // Return token value for END
END\.       { return 5;     } // Return token value for END.
INTEGER     { return 6;     } // Return token value for INTEGER
FOR         { return 7;     } // Return token value for FOR
READ        { return 8;     } // Return token value for READ
WRITE       { return 9;     } // Return token value for WRITE
TO          { return 10;    } // Return token value for TO
DO          { return 11;    } // Return token value for DO
\;          { return 12;    } // Return token value for ;
\:          { return 13;    } // Return token value for :
\,          { return 14;    } // Return token value for ,
\:\=        { return 15;    } // Return token value for :=
\+          { return 16;    } // Return token value for +
\-          { return 17;    } // Return token value for -
\*          { return 18;    } // Return token value for *
DIV         { return 19;    } // Return token value for DIV
\(          { return 20;    } // Return token value for (
\)          { return 21;    } // Return token value for )
{id}        { yylval = (void*)install22(); return 22; } // Handle IDs
{int}       { yylval = (void*)install_num(); return 23; } // Handle integers
.           { /* Handle invalid tokens as needed */ }
%%

/* yywrap function definition */
int yywrap(void) {
    return 1; // Default implementation returns 1
}

/* Install function for identifiers */
void* install22() {
    symbol* sym = search_symbol(yytext);
    if (sym == NULL)
        sym = insert_symbol(yytext, '^'); // Insert identifier into symbol table
    return sym;
}

/* Install function for numbers */
void* install_num() {
    symbol* sym = search_symbol(yytext);
    if (sym == NULL)
        sym = insert_symbol(yytext, '#'); // Insert number into symbol table
    return sym;
}

/* Main function for lexing and parsing */
int main() {
    
    // Initialize symbol table
    init_symbol_table();

    // Open a file for writing
    ofstream outfile("output.txt");
    if (!outfile.is_open()) {
        cerr << "Error opening file!" << endl;
        return 1;
    }
    outfile << "Line\tSymbol type\t\tSymbol Specifier\n";

    int token;
    int prev_line = 0;

    // Parse tokens from input program
    token = yylex();
    while (token) {
        // Print line number to file
        if (line == 0) {
            line++;
            outfile << line << "\t\t";
        } else {
            if (line == prev_line)
                outfile << " " << "\t\t";
            else
                outfile << line << "\t\t";
        }

        // Print token id, type, and specifier to file
        symbol* sym = nullptr;
        if (token == 22 || token == 23)
            sym = (symbol*)yylval;
        if (sym != nullptr)
            outfile << "\t" << token << "\t\t\t" << sym->type << sym->specifier << endl;
        else
            outfile << "\t" << token << endl;

        prev_line = line;
        token = yylex();
    }

    // Close the file
    outfile.close();

    // Print the symbol table
    print_symbol_table();

    return 0;
}
