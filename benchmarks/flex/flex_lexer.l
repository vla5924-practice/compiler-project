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
{Indentation}      /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("Indentation"));
{Bool}             /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("Bool"));
{False}            /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("False"));
{Def}              /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("Def"));
{Int}              /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("Int"));
{Float}            /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("Float"));
{Str}              /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("Str"));
{If}               /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("If"));
{Else}             /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("Else"));
{Elif}             /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("Elif"));
{Range}            /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("Range"));
{While}            /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("While"));
{For}              /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("For"));
{Break}            /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("Break"));
{Import}           /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("Import"));
{Continue}         /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("Continue"));
{Return}           /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("Return"));
{Or}               /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("Or"));
{And}              /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("And"));
{Not}              /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("Not"));
{In}               /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("In"));
{True}             /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("True"));
{None}             /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("None"));
{Class}            /*printf("%s", yytext);*/ values[last_index++] = simple_hash(yytext, sizeof("Class"));

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
    char* new_string = (char*)malloc(size);
    for (int i = 0; i < size; i++) {
        h = 42 * h + s[i];
        new_string[i] = s[i];
    }

    return size;
}

int main(int argc, char **argv)
{
;  /* skip over program name */
    if ( argc > 1 )
            yyin = fopen( argv[2], "r" );
    else
            yyin = stdin;
    printf("%d\n", argc);
    printf("Open: %s\n", argv[0]);
    printf("Open: %s\n", argv[1]);
    printf("Open: %s\n", argv[2]);
    //fp = fopen(argv[2], "w");
    FILE* fp_log = fopen(argv[3], "w");
    values = (unsigned int*)malloc(atoi(argv[1]) * sizeof(unsigned int));
    double start = 0.0; 
    double end = 0.0;
    start = omp_get_wtime();
    yylex();
    end = omp_get_wtime(); 
    char save_string[1024];
    printf("lexerflex Elapsed time: %.*f \n", 6, (end - start) * 1000);
    sprintf(save_string, "lexerflex Elapsed time: %f \n", (end - start) * 1000);
    fputs(save_string, fp_log);
    fclose(fp_log);
    return 0;
}
