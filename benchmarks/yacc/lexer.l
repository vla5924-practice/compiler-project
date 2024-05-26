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

identifier      [_a-zA-Z][_a-zA-Z0-9]*
defined         "and"|"elif"|"global"|"or"|"assert"|"else"|"if"|"except"|"pass"|"break"|"print"|"exec"|"in"|"raise"|"continue"|"finally"|"is"|"return"|"for"|"lambda"|"try"|"del"|"not"|"while"
quote1          "\'"[^'\\]*"\'"
quote2          "\""[^"\\]*"\""

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
{defined}       {
                    yylval = yytext;
                    #ifdef DEBUG 
                        cout << yytext << endl;
                    #endif
                    return DEFINED;
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
class           {
                    yylval = yytext;
                    return CLASS;
                }
def             {
                    yylval = yytext;
                    return DEF;
                }

":"             yylval = yytext; return COLON;
"."             yylval = yytext; return DOT;
","             yylval = yytext; return COMMA;
"("             yylval = yytext; return LBRACE;
")"             yylval = yytext; return RBRACE;
"*"             yylval = yytext; return STAR;

{identifier}    {
                    yylval = yytext; 
                    #ifdef DEBUG 
                        cout << yytext << endl; 
                    #endif
                    return ID;
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
{quote1}        {
                    yylval = yytext;
                    #ifdef DEBUG
                        cout << "MSG: " << yytext << endl;
                    #endif
                    return MESSAGE;
                }
{quote2}        {
                    yylval = yytext;
                    #ifdef DEBUG
                        cout << "MSG: " << yytext << endl;
                    #endif
                    return MESSAGE;
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
