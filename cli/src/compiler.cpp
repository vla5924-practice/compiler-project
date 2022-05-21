#include "compiler.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>

#include <argparse/argparse.hpp>
#include <backend/lexer/lexer.hpp>
#include <backend/parser/parser.hpp>
#include <backend/preprocessor/preprocessor.hpp>
#include <backend/semantizer/semantizer.hpp>
#include <utils/source_files.hpp>

#ifdef ENABLE_IR_GENERATOR
#include <ir_generator/ir_generator.hpp>
#endif

#include "dumping.hpp"

#define VERBOSE(EXPR)                                                                                                  \
    if (verbose) {                                                                                                     \
        EXPR;                                                                                                          \
    }

using lexer::Lexer;
using parser::Parser;
using preprocessor::Preprocessor;
using semantizer::Semantizer;
using utils::SourceFile;

#ifdef ENABLE_IR_GENERATOR
using ir_generator::IRGenerator;
#endif

namespace {

argparse::ArgumentParser createArgumentParser() {
    argparse::ArgumentParser parser("cli", "1.0");
    parser.add_argument("-h", "--help").help("show help message and exit").default_value(false).implicit_value(true);
    parser.add_argument("-v", "--verbose").help("print info messages").default_value(false).implicit_value(true);
#ifdef ENABLE_IR_GENERATOR
    parser.add_argument("-c", "--compile")
        .help("produce an executable instead of LLVM IR code")
        .default_value(false)
        .implicit_value(true);
    parser.add_argument("--clang")
        .help("path to clang executable (required if --compile argument is set)")
        .default_value("clang");
    parser.add_argument("--llc")
        .help("path to llc executabe (required if --compile argument is set)")
        .default_value("llc");
    parser.add_argument("-o", "--output").help("output file");
#endif
    parser.add_argument("FILES").required().remaining();
    return parser;
}

#ifdef ENABLE_IR_GENERATOR
std::filesystem::path createTemporaryDirectory() {
    auto tempDir = std::filesystem::temp_directory_path();
    std::random_device dev;
    std::mt19937 gen(dev());
    std::uniform_int_distribution<uint64_t> dist(0);
    std::filesystem::path path;
    for (int i = 0; i < 1000; i++) {
        std::stringstream dirName;
        dirName << std::hex << dist(gen);
        path = tempDir / dirName.str();
        if (std::filesystem::create_directory(path))
            return path;
    }
    throw std::runtime_error("could not find non-existing directory");
}

std::string wrapQuotes(const std::string &str) {
    if (str.front() == '"' && str.back() == '"')
        return str;
    return "\"" + str + "\"";
}

std::string generateLlcCommand(const std::string &llcBin, const std::filesystem::path &llFile,
                               const std::filesystem::path &objFile) {
    std::stringstream cmd;
    cmd << wrapQuotes(llcBin) << " -filetype=obj " << wrapQuotes(llFile.string()) << " -o "
        << wrapQuotes(objFile.string());
    return cmd.str();
}

std::string generateClangCommand(const std::string &clangBin, const std::filesystem::path &objFile,
                                 const std::filesystem::path &exeFile) {
    std::stringstream cmd;
    cmd << wrapQuotes(clangBin) << ' ' << wrapQuotes(objFile.string()) << " -o " << wrapQuotes(exeFile.string());
    return cmd.str();
}
#endif

} // namespace

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

    SourceFile source;
    for (const std::string &path : files) {
        SourceFile other = utils::readFile(path);
        source.insert(source.end(), std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()));
        VERBOSE(std::cout << "Read file " << path << std::endl);
    }

    ast::SyntaxTree tree;

    try {
        VERBOSE(std::cout << std::endl << "PREPROCESSOR:" << std::endl);
        source = Preprocessor::process(source);
        VERBOSE(std::cout << dumping::dump(source) << std::endl);

        VERBOSE(std::cout << "LEXER:" << std::endl);
        auto tokens = Lexer::process(source);
        VERBOSE(std::cout << dumping::dump(tokens) << std::endl);

        VERBOSE(std::cout << "PARSER:" << std::endl);
        tree = Parser::process(tokens);
        VERBOSE(tree.dump(std::cout));

        VERBOSE(std::cout << "SEMANTIZER:" << std::endl);
        Semantizer::process(tree);
        VERBOSE(tree.dump(std::cout));
    } catch (ErrorBuffer &errors) {
        std::cerr << errors.message();
        return 3;
    }

#ifdef ENABLE_IR_GENERATOR
    VERBOSE(std::cout << std::endl << "IR GENERATOR:" << std::endl);
    IRGenerator generator("module");
    generator.process(tree);
    VERBOSE(generator.dump());

    if (!program.is_used("--output"))
        return 0;

    bool compile = program.get<bool>("--compile");
    std::string output = program.get<std::string>("--output");

    if (!compile) {
        generator.writeToFile(output);
        return 0;
    }

    try {
        std::filesystem::path tempDir = createTemporaryDirectory();
        const auto llFile = tempDir / "out.ll";
        generator.writeToFile(llFile.string());
        const auto objFile = tempDir / "out.obj";
        const auto exeFile = tempDir / "out.exe";
        std::string llcCmd = generateLlcCommand(program.get<std::string>("--llc"), llFile, objFile);
        std::string clangCmd = generateClangCommand(program.get<std::string>("--clang"), objFile, exeFile);
        bool cmdFailed = (std::system(llcCmd.c_str()) || std::system(clangCmd.c_str()));
        if (!cmdFailed)
            std::filesystem::copy_file(exeFile, output);
        std::filesystem::remove_all(tempDir);
        if (cmdFailed)
            return 3;
    } catch (std::exception &e) {
        std::cerr << e.what();
        return 3;
    }
#endif

    return 0;
}
