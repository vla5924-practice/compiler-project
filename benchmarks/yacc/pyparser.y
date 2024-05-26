%{
///////////////////////////////////////////////////////////////////////////////
// This file is the part of SourceAnalyzer source codes.                     //
// SourceAnalyzer is a program that search out a call-graph of               //
// given source code. See <http://trac-hg.assembla.com/SourceAnalyzer>       //
// Copyright (C) 2008-2009 SourceAnalyzer contributors                       //
//                                                                           //
// This program is free software: you can redistribute it and/or modify      //
// it under the terms of the GNU General Public License as published by      //
// the Free Software Foundation, either version 3 of the License,            //
// any later version.                                                        //
//                                                                           //
// This program is distributed in the hope that it will be useful,           //
// but WITHOUT ANY WARRANTY; without even the implied warranty of            //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             //
// GNU General Public License for more details.                              //
//                                                                           //
// You should have received a copy of the GNU General Public License         //
// along with this program.  If not, see <http://www.gnu.org/licenses/>.     //
///////////////////////////////////////////////////////////////////////////////

    #include <cstdio>
    #include <string>
    #include <iostream>
    #include <stack>
    #include <math.h>
    #include <stdio.h>
    #include <omp.h>
    // extern "C" {
    //   #include "sa3parsers.h"
    // }
    
    using namespace std;
    #define YYSTYPE string
    #define YYERROR_VERBOSE 1
    
    typedef pair <string, int> meta_data;
    typedef stack <meta_data> meta_stack;

    static meta_stack class_st;
    static meta_stack func_st;
    static string func_name = "";
    static string call_params = "";
    static string file_name = "";
    
    // sa3cg cg = NULL;

    // remove all instructions (meta data -> string) from stack 
    // with largest indent than current instruction has
    static void clean_stack( meta_stack& stack, int indent );

    int  wrapRet = 1;
    
    int yylex(void);
    extern "C" {
        int yywrap( void ) {
            return wrapRet;
        }
    }
    void yyerror(const char *str) {
        //#ifdef DEBUG
          //  DEBUGMSG( line << ": SA Python Parser: " << str );
        //#endif
    }    
    extern FILE* yyin;   

    static const string type = "undefined";
    static bool isInteractive;
    int main(int argc, char **argv);
    static string get_call_function_name_from_call_chain(string call_chain);
%}

%token CLASS DEFINED COLON DOT LBRACE RBRACE ID OTHER DEF COMMA STAR MESSAGE
%start input

%left RBRACE
%left LBRACE

%%
input: /* empty */
     | input class_def
     | input func_def
     | input calls_chain
     | input error
;

/* CLASS */
class_def: CLASS classname inheritance COLON suite
    {
        int indent = @1.last_column;
        meta_data new_class($2, indent);

        clean_stack( class_st, indent );
        class_st.push( new_class );

        #ifdef DEBUG
            cout //<< "[" << indent << "] "  
                 << @$.first_line
                 << " >> CLASS: " 
                 << $2            << "("
                 << $3            << ")"
                 << endl;
        #endif
    }
;
classname: ID
            {
                $$ = $1;
            }
;
inheritance: /* empty */
            {
                $$ = "";
            }
           | LBRACE class_args_list RBRACE
            {
                $$ = $2;
            }
;
class_args_list: /* empty */
                {
                    $$ = "";
                }
               | class_arg
                {
                    $$ = $1;
                }
;
class_arg:  dotted_name
          | class_arg COMMA dotted_name
            {
                $$ += $2 + $3;
            }
;
/* end of CLASS */

/* FUNCTION */
func_def: DEF funcname LBRACE func_args_list RBRACE COLON suite
        {
            string fnc_name = $2;
            int indent = @1.last_column;
              
            clean_stack( class_st, indent );
            meta_stack tmp_class_st(class_st);

            while (!tmp_class_st.empty())
            {
                // Delete comment to turn on full function name:
                //    TestClass.test_function
                // fnc_name = tmp_class_st.top().first + "." + fnc_name;
                tmp_class_st.pop();
            }

            // replace fnc_name for $2 for less information 
            //   about function location
            meta_data new_func(fnc_name, indent);
            clean_stack( func_st, indent );
            func_st.push( new_func );

            #ifdef DEBUG
                cout //<< "[" << indent << "] " 
                     << @$.first_line << " >> FUNC:  " 
                     << fnc_name      << "("
                     << $4            << ")"
                     << endl;
            #endif

            // int ret = sa3adddecl (cg, (char*) type.c_str(), (char*) fnc_name.c_str(), (char*) $4.c_str());
            // if (0 != ret) {
            //     cerr << "ERROR: sa3adddecl failed with " << ret << endl;
            // }
        }
;
funcname: ID
        {
            $$ = $1;
        }
;
func_args_list: /* empty */
                {
                    $$ = "";
                }
              | func_arg
                {
                    $$ = $1;
                }
;
func_arg: dotted_name
        | star_arg
        | calls_chain
        | func_arg OTHER
            {
                $$ += $2;
            }
        | func_arg COMMA
            {
                $$ += $2;
            }
        | func_arg dotted_name
            {
                $$ += $2;
            }
        | func_arg star_arg
            {
                $$ += $2;
            }
        | func_arg MESSAGE
            {
                $$ += $2;
            }
        | func_arg calls_chain
            {
                $$ += $2;
            }
;
star_arg: STAR ID
            {
                $$ = $1 + $2;
            }
        | STAR STAR ID
            {
                $$ = $1 + $2 + $3;
            }
;
/* end of FUNCTION */

suite:
;

/* FUNCTION CALL */
calls_chain: func_call
            {
                string cd = get_call_function_name_from_call_chain($$);
                #ifdef DEBUG
                     cout //<< "[" << @$.last_column << "] " 
                          << @$.first_line
                          << " Function1: " << func_name 
                          << " >> CALL: "  << cd
                          << " >> PARAM: " << call_params
                          << endl;
                #endif

                // int ret = sa3addcall (cg, (char*)func_name.c_str(), (char*)cd.c_str());
                // if (0 != ret) {
                //     cerr << "ERROR: sa3adddecl failed with " << ret << endl;
                // }
            }
           | calls_chain DOT func_call
            {
                $$ += $2 + $3;
                string cd = get_call_function_name_from_call_chain($$);

                #ifdef DEBUG
                     cout //<< "[" << @$.last_column << "] " 
                          << @$.first_line 
                          << " Function2: " << func_name 
                          << " >> CALL: "  << cd
                          << " >> PARAM: " << call_params << endl;                     
                #endif

                // int ret = sa3addcall (cg, (char*)func_name.c_str(), (char*)cd.c_str());
                // if (0 != ret) {
                //     cerr << "ERROR: sa3adddecl failed with " << ret << endl;
                // }
            }
;
func_call: dotted_name func_call_params
            {
                bool isFirst = true;

                func_name = "";
                // if func_call_params are in more than 1 line
                //   then indent can be unexpected
                //   but @1 determines it correctly
                int indent = @1.last_column;

                clean_stack(func_st, indent);
                meta_stack tmp_func_st(func_st);

                while (!tmp_func_st.empty())
                {
                    if(true == isFirst)
                    {
                        func_name = tmp_func_st.top().first;
                        tmp_func_st.pop();
                        isFirst = false;
                        continue;
                    }
                    // func_name = tmp_func_st.top().first + "." + func_name;
                    tmp_func_st.pop();
                }

                if (func_name == "") {
                    func_name = "__main__";
                }

                $$ = $1 + $2;
            }
;
dotted_name: ID
           | dotted_name DOT ID
            {
                $$ += $2 + $3;
            }
;
func_call_params: LBRACE RBRACE
                    {
                        call_params = "";
                        $$ = $1 + $2;
                    }
                | LBRACE call_params RBRACE
                    {
                        call_params = $2;
                        $$ = $1 + $2 + $3;
                    }
;
call_params: OTHER
           | DEFINED
           | MESSAGE
           | dotted_name
           | STAR
           | calls_chain
           | func_call_params
           | call_params DEFINED
            {
                $$ += $2;
            }
           | call_params MESSAGE
            {
                $$ += $2;
            }
           | call_params dotted_name
            {
                $$ += $2;
            }
           | call_params OTHER
            {
                $$ += $2;
            }
           | call_params calls_chain
            {
                $$ += $2;
            }
           | call_params COMMA
            {
                $$ += $2;
            }
           | call_params COLON
            {
                $$ += $2;
            }
           | call_params STAR
            {
                $$ += $2;
            }
           | call_params func_call_params
            {
                $$ += $2;
            }
;
/* end of FUNCTION CALL */

%%

static void clean_stack(meta_stack& stack, int indent)
{
    while(!stack.empty())
    {
        if(indent > stack.top().second)
            break;
        stack.pop();
    }
}

static string get_call_function_name_from_call_chain(string call_chain)
{
    int braces = 0;
    string name = "";
    for (int i = call_chain.length()-1; i > 0; i--)
    {
        switch (call_chain[i])
        {
            case ')':   braces ++;
                        continue;
            case '(':   braces --;
                        if (braces == 0) {
                            for (int j = i-1; j >= 0; j--)
                            {
                                if (call_chain[j] != '.') {
                                    name = call_chain[j] + name;
                                }
                                else {
                                    return name;
                                }
                            }
                            return name;
                        }
            default: ;
        }
    }
    return name;
}

int main(int argc, char **argv)
{
    yyin = fopen( argv[1], "r" );
    double start = omp_get_wtime();
    int a = yyparse();
    double end = omp_get_wtime(); 
    printf("Flex Elapsed time: %.*f", 6,(end - start) * 1000);
    return a;
}