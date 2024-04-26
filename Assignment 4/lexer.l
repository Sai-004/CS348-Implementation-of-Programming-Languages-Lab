%{
	/* header files */
	#include<stdio.h>
	#include<stdlib.h>
	#include<string.h>
	#include "y.tab.h"     // Include the yacc/bison generated header file
	#include "symbol_table.c"    // Include the symbol table file for symbol table operations
%}

/* Define regular expression patterns */
delim	[ \t\n]
aplha	[A-Za-z]
digit	[0-9]
id		{aplha}({aplha}|{digit})*
int		{digit}+
real  	{digit}+(\.{digit}+)?(E[+-]?{digit}+)?

%%

\n		{ line++; }          	// Count lines
[ \t]	{ }                  	// Ignore whitespace
PROGRAM	{ return _PROGRAM ; }  	// Return token for PROGRAM
VAR		{ return _VAR ; }      	// Return token for VAR
BEGIN	{ return _BEGIN ; }    	// Return token for BEGIN
END		{ return _END ; }      	// Return token for END
END\.	{ return _END_DOT ; }  	// Return token for END followed by a period
INTEGER	{ return _INTEGER ; }  	// Return token for INTEGER
REAL	{ return _REAL ; }      // Return token for REAL
FOR		{ return _FOR ; }      	// Return token for FOR
READ	{ return _READ; }      	// Return token for READ
WRITE	{ return _WRITE ; }    	// Return token for WRITE
TO		{ return _TO; }        	// Return token for TO
DO		{ return _DO; }        	// Return token for DO
\;		{ return _SEMICOLON; }  // Return token for SEMICOLON
\:		{ return _COLON; }      // Return token for COLON
\,		{ return _COMMA; }      // Return token for COMMA
\:\=	{ return _ASSIGN; }     // Return token for ASSIGN
\+		{ return _ADD; }        // Return token for ADDITION
\-		{ return _SUB; }        // Return token for SUBTRACTION
\*		{ return _MUL; }        // Return token for MULTIPLICATION
DIV		{ return _DIV; }        // Return token for DIVISION
\(		{ return _OPEN_BRACE; }	// Return token for OPEN_BRACE
\)		{ return _CLOSE_BRACE;}	// Return token for CLOSE_BRACE
{id}	{                     		// Identifier encountered
          yylval.ID = install_id(); // Store the identifier
          return _ID;               // Return token for identifier
       }
{int}	{                           // Integer encountered
          yylval.NUMBER.integer = install_int();	// Store the integer
          yylval.NUMBER.type = 0;                 	// Set type to integer
          return _INT;                           	// Return token for integer
       }
{real}	{                           // Real number encountered
          yylval.NUMBER.real = install_real();   	// Store the real number
          yylval.NUMBER.type = 1;                 	// Set type to real
          return _FLOAT;                        	// Return token for real number
       }
.		{ printf("error: unexpected character syntax error, line number: %d\n", line + 1); }  // Error for unexpected characters
%%

/* Function definitions */

// Function to install an identifier
char* install_id() {
	symbol* sym = search(yytext);	// Search for the identifier in symbol table
	if(sym == NULL)
		sym = insert(yytext, 'V');  // If not found, insert it as a variable
	return strdup(yytext);         	// Return a duplicate of the identifier string
}

// Function to convert string to integer
int install_int() {
	return atoi(yytext);           	// Convert string to integer and return
}

// Function to convert string to float
float install_real() {
	return atof(yytext);           	// Convert string to float and return
}
