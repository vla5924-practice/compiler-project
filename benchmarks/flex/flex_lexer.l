%{
#include <math.h>
#include <stdio.h>
#include <omp.h>

FILE *fp;
unsigned int *values;
int last_index = 0;
unsigned int simple_hash(const char* s, int size);
%}

DIGIT       [0-9]
Identifier  [_a-zA-Z][_a-zA-Z0-9]*

Indentation     "    "
Eoe             [\n]
Arrow           "->"
Colon           ":"

Quote1          "\'"[^'\\]*"\'"
Quote2          "\""[^"\\]*"\""

Bool            "bool"
False           "False" 
Int             "int"
Float           "float" 
Str             "str"
If              "if" 
Else            "else"
Elif            "elif" 
Range           "range"
While           "while" 
For             "for"
Break           "break" 
Import          "import"
Continue        "continue" 
Return          "return" 
Or              "or"
And             "and" 
Not             "not"
In              "in" 
True            "True"
None            "None"
Class           "class"
Def             "def"

Mod             "%"
Dot             "."
RectRightBrace  "]"
Comma           ","
Assign          "="
Add             "+"
Sub             "-"
Mult            "*"
Div             "/"
Equal           "=="
NotEqual        "!="
Less            "<"
Greater         ">"
LessEqual       "<="
GreaterEqual    ">="
LeftBrace       "("
RightBrace      ")"
RectLeftBrace   "["

IntegerLiteral         {DIGIT}+
FloatLiteral           {DIGIT}+"."{DIGIT}*
StringLiteral          {Quote1}|{Quote2}
%%
{Indentation}      yytext; values[last_index++] = simple_hash(yytext, sizeof("Indentation"));
{Bool}             yytext; values[last_index++] = simple_hash(yytext, sizeof("Bool"));
{False}            yytext; values[last_index++] = simple_hash(yytext, sizeof("False"));
{Def}              yytext; values[last_index++] = simple_hash(yytext, sizeof("Def"));
{Int}              yytext; values[last_index++] = simple_hash(yytext, sizeof("Int"));
{Float}            yytext; values[last_index++] = simple_hash(yytext, sizeof("Float"));
{Str}              yytext; values[last_index++] = simple_hash(yytext, sizeof("Str"));
{If}               yytext; values[last_index++] = simple_hash(yytext, sizeof("If"));
{Else}             yytext; values[last_index++] = simple_hash(yytext, sizeof("Else"));
{Elif}             yytext; values[last_index++] = simple_hash(yytext, sizeof("Elif"));
{Range}            yytext; values[last_index++] = simple_hash(yytext, sizeof("Range"));
{While}            yytext; values[last_index++] = simple_hash(yytext, sizeof("While"));
{For}              yytext; values[last_index++] = simple_hash(yytext, sizeof("For"));
{Break}            yytext; values[last_index++] = simple_hash(yytext, sizeof("Break"));
{Import}           yytext; values[last_index++] = simple_hash(yytext, sizeof("Import"));
{Continue}         yytext; values[last_index++] = simple_hash(yytext, sizeof("Continue"));
{Return}           yytext; values[last_index++] = simple_hash(yytext, sizeof("Return"));
{Or}               yytext; values[last_index++] = simple_hash(yytext, sizeof("Or"));
{And}              yytext; values[last_index++] = simple_hash(yytext, sizeof("And"));
{Not}              yytext; values[last_index++] = simple_hash(yytext, sizeof("Not"));
{In}               yytext; values[last_index++] = simple_hash(yytext, sizeof("In"));
{True}             yytext; values[last_index++] = simple_hash(yytext, sizeof("True"));
{None}             yytext; values[last_index++] = simple_hash(yytext, sizeof("None"));
{Class}            yytext; values[last_index++] = simple_hash(yytext, sizeof("Class"));

{Mod}              values[last_index++] = simple_hash(yytext, sizeof("Mod"));
{Dot}              values[last_index++] = simple_hash(yytext, sizeof("Dot"));
{RectRightBrace}   values[last_index++] = simple_hash(yytext, sizeof("RectRightBrace"));
{Comma}            values[last_index++] = simple_hash(yytext, sizeof("Comma"));
{Assign}           values[last_index++] = simple_hash(yytext, sizeof("Assign"));
{Add}              values[last_index++] = simple_hash(yytext, sizeof("Add"));
{Sub}              values[last_index++] = simple_hash(yytext, sizeof("Sub"));
{Mult}             values[last_index++] = simple_hash(yytext, sizeof("Mult"));
{Div}              values[last_index++] = simple_hash(yytext, sizeof("Div"));
{Equal}            values[last_index++] = simple_hash(yytext, sizeof("Equal"));
{NotEqual}         values[last_index++] = simple_hash(yytext, sizeof("NotEqual"));
{Less}             values[last_index++] = simple_hash(yytext, sizeof("Less"));
{Greater}          values[last_index++] = simple_hash(yytext, sizeof("Greater"));
{LessEqual}        values[last_index++] = simple_hash(yytext, sizeof("LessEqual"));
{GreaterEqual}     values[last_index++] = simple_hash(yytext, sizeof("GreaterEqual"));
{LeftBrace}        values[last_index++] = simple_hash(yytext, sizeof("LeftBrace"));
{RightBrace}       values[last_index++] = simple_hash(yytext, sizeof("RightBrace"));
{RectLeftBrace}    values[last_index++] = simple_hash(yytext, sizeof("RectLeftBrace"));

{IntegerLiteral}         atoi(yytext); values[last_index++] = simple_hash(yytext, sizeof("IntegerLiteral"));
{FloatLiteral}           atof(yytext); values[last_index++] = simple_hash(yytext, sizeof("FloatLiteral"));

{Identifier}    {
                    values[last_index++] =  simple_hash(yytext, sizeof("Identifier")); 
                    #ifdef DEBUG 
                        cout << yytext << endl; 
                    #endif

                }
\"\"\"          {
                    /* skip messages in triple double quotes */
                    //yylval = skipMessage((string)("\"\"\""));
                    #ifdef DEBUG
                        cout << "MSG: " << yylval << endl;
                    #endif

                }
\'\'\'          {
                    /* skip messages in triple single quotes */
                    //yylval = skipMessage((string)("\'\'\'"));
                    #ifdef DEBUG
                        cout << "MSG: " << yylval << endl;
                    #endif

                }
{StringLiteral}        {
                    values[last_index++] = simple_hash(yytext, sizeof("StringLiteral"));
                    #ifdef DEBUG
                        cout << "MSG: " << yytext << endl;
                    #endif

                }

[ \t]+          ; /* skip whitespaces */
.               {
                    
                    #ifdef DEBUG 
                        cout << yytext << endl; 
                    #endif
                }

%%

/* User code*/


unsigned int simple_hash(const char* s, int size)
{
    unsigned int h = 0;
    for (int i = 0; i < size; i++)
        h = 42 * h + s[i];
    return h;
}

int main(int argc, char **argv)
{
    ++argv, --argc;  /* skip over program name */
    if ( argc > 0 )
            yyin = fopen( argv[0], "r" );
    else
            yyin = stdin;
    fp = fopen(argv[2], "w");
    values = (unsigned int*)malloc(atoi(argv[1]) * sizeof(unsigned int));
    double start = 0.0; 
    double end = 0.0;
    start = omp_get_wtime();
    yylex();
    end = omp_get_wtime(); 
    char save_string[1024];
    printf("Flex Elapsed time: %.*f \n", 6, (end - start) * 1000);
    sprintf(save_string, "Flex Elapsed time: %.*f \n", 6, (end - start) * 1000);
    fputs(save_string, fp);
    fclose(fp);
    return 0;
}