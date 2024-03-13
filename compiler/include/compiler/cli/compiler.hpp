#pragma once

class Compiler {
  public:
    Compiler() = delete;
    Compiler(const Compiler &) = delete;
    Compiler(Compiler &&) = delete;
    ~Compiler() = delete;

    static int exec(int argc, char *argv[]);
};
