#pragma once

#include <argparse/argparse.hpp>
#include <backend/stringvec.hpp>

class Compiler {
    static argparse::ArgumentParser createArgumentParser();
    static StringVec readFile(const std::string &path);

  public:
    Compiler() = delete;
    Compiler(const Compiler &) = delete;
    Compiler(Compiler &&) = delete;
    ~Compiler() = delete;

    static int exec(int argc, char *argv[]);
};
