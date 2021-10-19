#pragma once

#include <backend/lexer/lexer.hpp>
#include <backend/parser.hpp>
#include <backend/preprocessor.hpp>

class Compiler {
    static StringVec readFile(const std::string &path);

  public:
    Compiler() = delete;
    Compiler(const Compiler &) = delete;
    Compiler(Compiler &&) = delete;
    ~Compiler() = delete;

    static int exec(int argc, char *argv[]);
};
