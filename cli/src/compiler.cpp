#include "compiler.hpp"

#include <iostream>

#include <backend/stringvec.hpp>

int Compiler::exec(int argc, char *argv[]) {
    std::cout << "Compiler launched.\n";
    std::cout << std::endl;
    // source = readFile(pathtofile...);
    StringVec source = {
    "    ",
    "i12, j, k: int", 
    "   ",         
    "    i:=j + k;", 
    "   i:=i + 123;",
    "   i:= 123.123;",
    "   'weqweqwe'", "end."
    };
    source = Preprocessor::process(source);
    std::cout << source << std::endl;
    lexer::TokenList tokens = lexer::Lexer::process(source);
    std::cout << tokens << std::endl;

    return 0;
}
