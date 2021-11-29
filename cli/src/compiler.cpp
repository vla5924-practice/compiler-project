#include "compiler.hpp"

#include <fstream>
#include <iostream>

#include <backend/lexer/lexer.hpp>
#include <backend/parser/parser.hpp>
#include <backend/preprocessor/preprocessor.hpp>

#include "dumping.hpp"

using lexer::Lexer;
using parser::Parser;
using preprocessor::Preprocessor;

argparse::ArgumentParser Compiler::createArgumentParser() {
    argparse::ArgumentParser parser("cli", "1.0");
    parser.add_argument("-h", "--help").help("show help message and exit").default_value(false).implicit_value(true);
    parser.add_argument("-v", "--verbose").help("print info messages").default_value(false).implicit_value(true);
    parser.add_argument("-o", "--output").help("output file").default_value("a.out");
    parser.add_argument("FILES").required().remaining();
    return parser;
}

StringVec Compiler::readFile(const std::string &path) {
    std::ifstream file(path);
    std::string str;
    StringVec content;
    while (std::getline(file, str)) {
        content.push_back(str);
    }
    return content;
}

int Compiler::exec(int argc, char *argv[]) {
    argparse::ArgumentParser program = createArgumentParser();
    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    if (program.get<bool>("--help")) {
        std::cout << program;
        return 0;
    }

    bool verbose = program.get<bool>("--verbose");
    std::vector<std::string> files;
    try {
        files = program.get<std::vector<std::string>>("FILES");
        std::cout << files.size() << " file(s) provided:" << std::endl;
        for (auto &file : files)
            std::cout << "  " << file << std::endl;
    } catch (std::logic_error &e) {
        std::cerr << "No files provided" << std::endl;
        return 2;
    }

    StringVec source;
    for (const std::string &path : files) {
        StringVec other = readFile(path);
        source.insert(source.end(), std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()));
        if (verbose)
            std::cout << "Read file " << path << std::endl;
    }

    try {
        if (verbose)
            std::cout << std::endl << "PREPROCESSOR:" << std::endl;
        source = Preprocessor::process(source);
        if (verbose)
            std::cout << dumping::dump(source) << std::endl;

        if (verbose)
            std::cout << "LEXER:" << std::endl;
        auto tokens = Lexer::process(source);
        if (verbose)
            std::cout << dumping::dump(tokens) << std::endl;

        if (verbose)
            std::cout << "PARSER:" << std::endl;
        auto tree = Parser::process(tokens);
        if (verbose)
            std::cout << dumping::dump(tree) << std::endl;
    } catch (ErrorBuffer &errors) {
        std::cerr << errors.message();
        return 3;
    }

    return 0;
}
