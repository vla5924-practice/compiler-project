%{
#include <math.h>
#include "token_types.hpp"
#include <stdio.h>

#define WRITE_TO_FILE
#define PRINT

FILE *fp;
%}

/*Definition block*/
DIGIT       [0-9]
IDENTIFIER  [_a-zA-Z][_a-zA-Z0-9]*
KEYWORD     "bool"|"int"|"str"|"else"|"range"|"for"|"import"|"def"|"or"|"not"|"True"|"False"|"float"|"if"|"elif"|"while"|"break"|"continue"|"return"|"and"|"in"|"None"
OPERATOR    "%"|","|"-"|"=="|">"|"("|"."|"="|"*"|"!="|"<="|")"|"]"|"+"|"/"|"<"|">="|"["
SPECIAL     ":"|"->"|"    "|[\n]
QUOTE1      "\'"[^'\\]*"\'"
QUOTE2      "\""[^"\\]*"\""

/*Rules blok*/
INTEGER     {DIGIT}+
FLOAT       {DIGIT}+"."{DIGIT}*
STRINGS     {QUOTE1}|{QUOTE2}

/*States rections*/
%%
{INTEGER} {
    #ifdef PRINT
        printf( "An integer: %s (%d)\n", yytext, atoi( yytext ) );
    #endif

    #ifdef WRITE_TO_FILE
        fputs(yytext, fp);
        fputs(" : IntegerLiteral\n", fp);
    #endif
}

{FLOAT} {
    #ifdef PRINT
        printf( "A float: %s (%g)\n", yytext, atof( yytext ) );
    #endif

    #ifdef WRITE_TO_FILE
        fputs(yytext, fp);
        fputs(" : FloatingPointLiteral\n", fp);
    #endif
}

{KEYWORD} {
    #ifdef PRINT
        printf( "A keyword: %s\n", yytext);
    #endif

    #ifdef WRITE_TO_FILE
        fputs(yytext, fp);
        fputs(" : Keyword\n", fp);
    #endif
}

{IDENTIFIER} {
    #ifdef PRINT
        printf( "An identifier: %s\n", yytext );
    #endif

    #ifdef WRITE_TO_FILE
        fputs(yytext, fp);
        fputs(" : Identifier\n", fp);
    #endif
}

{OPERATOR} {
    #ifdef PRINT
        printf( "An operator: %s\n", yytext );
    #endif

    #ifdef WRITE_TO_FILE
        fputs(yytext, fp);
        fputs(" : Operator\n", fp);
    #endif
}

{STRINGS} {
    #ifdef PRINT
        printf( "An string: %s\n", yytext );
    #endif

    #ifdef WRITE_TO_FILE
        fputs(yytext, fp);
        fputs(" : StringLiteral\n", fp);
    #endif
}

{SPECIAL} {
    #ifdef PRINT
        printf( "An Special: %s\n", yytext );
    #endif

    #ifdef WRITE_TO_FILE
        fputs(yytext, fp);
        fputs(" : Special\n", fp);
    #endif
}

[ ]               printf( "Space: %s\n", yytext );
"{"[^}\n]*"}"     printf( "Unrecognized character: %s\n", yytext );/* eat up one-line comments */



.           printf( "Unrecognized character: %s\n", yytext );

%%

/* User code*/

main(int argc, char **argv)
{
    ++argv, --argc;  /* skip over program name */
    if ( argc > 0 )
            yyin = fopen( argv[0], "r" );
    else
            yyin = stdin;

    fp = fopen("output.log", "w");

    yylex();
    fclose(fp);
}