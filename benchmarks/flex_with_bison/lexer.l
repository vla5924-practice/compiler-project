%{
    #include <string>
    #include <iostream>

    using namespace std;
    //#define DEBUG
    #define YYSTYPE string

    #include "pyparser.tab.h"
    
    extern YYLTYPE yylloc;
    #define YY_USER_INIT yylloc.first_line=1;
    #define SPACES_PER_INDENT 4
    
    static bool isNewLine = false;

    static string skipMessage(string _ch);
    static void skipComments();
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
Definition      "def"
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

Integer         {DIGIT}+
Float           {DIGIT}+"."{DIGIT}*
String          {Quote1}|{Quote2}

%%
\n\r|\r\n|\n|\r {
                    /* skip empty line */
                    yylloc.first_line++;
                    yylloc.last_column = 0;
                    isNewLine = true;
                }
"#"             {
                    /* skip comment */
                    skipComments();
                }
^[ ]+           {
                    if (isNewLine == true && 0 == yyleng % SPACES_PER_INDENT)
                        yylloc.last_column = yyleng / SPACES_PER_INDENT;

                    isNewLine = false;
                }
^[\t]+          {
                    if (isNewLine == true)
                        yylloc.last_column = yyleng;

                    isNewLine = false;
                }

{Bool}            yylval = yytext; return BOOL;
{False}           yylval = yytext; return FALSE;
{Def}             yylval = yytext; return DEF;
{Int}             yylval = yytext; return INT;
{Float}           yylval = yytext; return FLOAT;
{Str}             yylval = yytext; return STR;
{If}              yylval = yytext; return IF;
{Else}            yylval = yytext; return ELSE;
{Elif}            yylval = yytext; return ELIF;
{Range}           yylval = yytext; return RANGE;
{While}           yylval = yytext; return WHILE;
{For}             yylval = yytext; return FOR;
{Break}           yylval = yytext; return BREAK;
{Import}          yylval = yytext; return IMPORT;
{Continue}        yylval = yytext; return CONTINUE;
{Definition}      yylval = yytext; return DEFENITION;
{Return}          yylval = yytext; return RETURN;
{Or}              yylval = yytext; return OR;
{And}             yylval = yytext; return AND;
{Not}             yylval = yytext; return NOT;
{In}              yylval = yytext; return IN;
{True}            yylval = yytext; return TRUE;
{None}            yylval = yytext; return NONE;
{Class}           yylval = yytext; return CLASS;

{Mod}             yylval = yytext; return MOD;
{Dot}             yylval = yytext; return DOT;
{RectRightBrace}  yylval = yytext; return RECTRIGHTBRACE;
{Comma}           yylval = yytext; return COMMA;
{Assign}          yylval = yytext; return ASSIGN;
{Add}             yylval = yytext; return ADD;
{Sub}             yylval = yytext; return SUB;
{Mult}            yylval = yytext; return MULT;
{Div}             yylval = yytext; return DIV;
{Equal}           yylval = yytext; return EQUAL;
{NotEqual}        yylval = yytext; return NOTEQUAL;
{Less}            yylval = yytext; return LESS;
{Greater}         yylval = yytext; return GREATER;
{LessEqual}       yylval = yytext; return LESSEQUAL;
{GreaterEqual}    yylval = yytext; return GREATEREQUAL;
{LeftBrace}       yylval = yytext; return LEFTBRACE;
{RightBrace}      yylval = yytext; return RIGHTBRACE;
{RectLeftBrace}   yylval = yytext; return RECTLEFTBRACE;

{Integer}         yylval = atoi(yytext); return INTEGERLITERAL;
{Float}           yylval = atof(yytext); return FLOATLITERAL;

{Identifier}    {
                    yylval = yytext; 
                    #ifdef DEBUG 
                        cout << yytext << endl; 
                    #endif
                    return IDENTIFIER;
                }
\"\"\"          {
                    /* skip messages in triple double quotes */
                    yylval = skipMessage((string)("\"\"\""));
                    #ifdef DEBUG
                        cout << "MSG: " << yylval << endl;
                    #endif
                    return MESSAGE;
                }
\'\'\'          {
                    /* skip messages in triple single quotes */
                    yylval = skipMessage((string)("\'\'\'"));
                    #ifdef DEBUG
                        cout << "MSG: " << yylval << endl;
                    #endif
                    return MESSAGE;
                }
{String}        {
                    yylval = yytext;
                    #ifdef DEBUG
                        cout << "MSG: " << yytext << endl;
                    #endif
                    return STRING;
                }

[ \t]+          ; /* skip whitespaces */
.               {
                    yylval = yytext;
                    #ifdef DEBUG 
                        cout << yytext << endl; 
                    #endif
                    return OTHER;
                }
%%

static string skipMessage(string _ch){
    string ch;
    string str = _ch;
    _ch = _ch[0];
    bool skip = false;
    int count = 1;
    for(;;ch = yyinput()) {
        str += ch;
        if (ch == _ch){
            if (count == 3)
                return str;
            else count++;
        } else
            count = 1;

        if ("\n" == ch || "\r" == ch)
            yylloc.first_line++;
    }
}

static void skipComments(){
    for(char ch = yyinput();;ch = yyinput()) {
        if ('\0' == ch || '\n' == ch || '\r' == ch) {
            yylloc.first_line++;
            break;
        }
    }
}

main( argc, argv )
int argc;
char **argv;
{
    ++argv, --argc;  /* skip over program name */
    if ( argc > 0 )
            yyin = fopen( argv[0], "r" );
    else
            yyin = stdin;

    yylex();
}
