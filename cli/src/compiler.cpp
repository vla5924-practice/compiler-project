#include "compiler.hpp"

#include <iostream>

#include <backend/lexer/lexer.hpp>
#include <backend/parser/parser.hpp>
#include <backend/preprocessor/preprocessor.hpp>

int Compiler::exec(int argc, char *argv[]) {
    std::cout << "Compiler launched.\n";
    std::cout << std::endl;
    // source = readFile(pathtofile...);
    StringVec source = {"var i12, j, k: integer;", "begin",         "   i:=j + k;", "   i:=i + 123;",
                        "   i:= 123.123;",         "   'weqweqwe'", "end."};
    source = preprocessor::Preprocessor::process(source);
    std::cout << source << std::endl;
    lexer::TokenList tokens = lexer::Lexer::process(source);
    std::cout << tokens << std::endl;

    return 0;
}
