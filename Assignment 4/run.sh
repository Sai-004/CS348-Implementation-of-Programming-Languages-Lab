lex lexer.l
yacc -d parser.y
cc lex.yy.c y.tab.c
