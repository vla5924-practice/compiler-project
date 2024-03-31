#include "compiler.hpp"

#include <iostream>
#include <string>
#include <vector>

#ifdef ENABLE_CODEGEN
#include <cstdlib>
#include <filesystem>
#include <random>
#include <sstream>
#endif

#include <argparse/argparse.hpp>

#include "compiler/backend/ast/optimizer/optimizer.hpp"
#include "compiler/backend/ast/semantizer/semantizer.hpp"
#include "compiler/frontend/lexer/lexer.hpp"
#include "compiler/frontend/parser/parser.hpp"
#include "compiler/frontend/preprocessor/preprocessor.hpp"
#include "compiler/utils/source_files.hpp"

#ifdef ENABLE_CODEGEN
#include "compiler/codegen/ir_generator.hpp"
#endif

#include "dumping.hpp"
#include "logger.hpp"

using lexer::Lexer;
using optimizer::Optimizer;
using parser::Parser;
using preprocessor::Preprocessor;
using semantizer::Semantizer;
using utils::SourceFile;

#ifdef ENABLE_CODEGEN
using ir_generator::IRGenerator;
#endif

namespace {

const char *const ARG_HELP = "--help";
const char *const ARG_VERBOSE = "--verbose";
const char *const ARG_LOG = "--log";
const char *const ARG_OPTIMIZE = "--optimize";
const char *const ARG_FILES = "FILES";

#ifdef ENABLE_CODEGEN
const char *const ARG_COMPILE = "--compile";
const char *const ARG_CLANG = "--clang";
const char *const ARG_LLC = "--llc";
const char *const ARG_OUTPUT = "--output";
#endif

#ifdef ENABLE_CODEGEN
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
    argparse::ArgumentParser program("compiler", "1.0", argparse::default_arguments::none);
    program.add_argument("-h", ARG_HELP).help("show help message and exit").default_value(false).implicit_value(true);
    program.add_argument("-v", ARG_VERBOSE).help("print info messages").default_value(false).implicit_value(true);
    program.add_argument("-l", ARG_LOG).help("log file (stages output will be saved if provided)");
    program.add_argument("-O", ARG_OPTIMIZE).help("run optimization pass").default_value(false).implicit_value(true);
#ifdef ENABLE_CODEGEN
    program.add_argument("-c", ARG_COMPILE)
        .help("produce an executable instead of LLVM IR code")
        .default_value(false)
        .implicit_value(true);
    program.add_argument(ARG_CLANG)
        .help("path to clang executable (required if --compile argument is set)")
        .default_value("clang");
    program.add_argument(ARG_LLC)
        .help("path to llc executable (required if --compile argument is set)")
        .default_value("llc");
    program.add_argument("-o", ARG_OUTPUT).help("output file");
#endif
    program.add_argument(ARG_FILES).help("source files (separated by spaces)").required();

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    if (program.get<bool>(ARG_HELP)) {
        std::cout << program;
        return 0;
    }

    Logger logger;
    bool verbose = program.get<bool>(ARG_VERBOSE);
    if (verbose)
        logger.setStdoutEnabled(true);
    if (program.is_used(ARG_LOG))
        logger.setOutputFile(program.get<std::string>(ARG_LOG));

    std::vector<std::string> files;
    try {
        files = program.get<std::vector<std::string>>(ARG_FILES);
        logger << files.size() << " file(s) provided:" << std::endl;
        for (auto &file : files)
            logger << "  " << file << std::endl;
    } catch (std::logic_error &e) {
        std::cerr << "No files provided" << std::endl;
        return 2;
    }

    SourceFile source;
    for (const std::string &path : files) {
        SourceFile other = utils::readFile(path);
        source.insert(source.end(), std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()));
        logger << "Read file " << path << std::endl;
    }

    ast::SyntaxTree tree;

    try {
        logger << std::endl << "PREPROCESSOR:" << std::endl;
        source = Preprocessor::process(source);
        logger << dumping::dump(source) << std::endl;

        logger << "LEXER:" << std::endl;
        auto tokens = Lexer::process(source);
        logger << dumping::dump(tokens) << std::endl;

        logger << "PARSER:" << std::endl;
        tree = Parser::process(tokens);
        logger << tree.dump();

        logger << "SEMANTIZER:" << std::endl;
        Semantizer::process(tree);
        logger << tree.dump();

        if (program.get<bool>(ARG_OPTIMIZE)) {
            logger << "OPTIMIZER:" << std::endl;
            Optimizer::process(tree);
            logger << tree.dump();
        }
    } catch (ErrorBuffer &errors) {
        std::cerr << errors.message();
        return 3;
    }

#ifdef ENABLE_CODEGEN
    logger << std::endl << "IR GENERATOR:" << std::endl;
    IRGenerator generator("module");
    generator.process(tree);
    if (verbose) {
        generator.dump();
    }

    if (!program.is_used(ARG_OUTPUT))
        return 0;

    bool compile = program.get<bool>(ARG_COMPILE);
    std::string output = program.get<std::string>(ARG_OUTPUT);

    if (!compile) {
        generator.writeToFile(output);
        return 0;
    }

    std::string llcBin = program.is_used(ARG_LLC) ? program.get<std::string>(ARG_LLC) : "llc";
    std::string clangBin = program.is_used(ARG_CLANG) ? program.get<std::string>(ARG_CLANG) : "clang";

    try {
        std::filesystem::path tempDir = createTemporaryDirectory();
        const auto llFile = tempDir / "out.ll";
        generator.writeToFile(llFile.string());
        const auto objFile = tempDir / "out.obj";
        const auto exeFile = tempDir / "out.exe";
        std::string llcCmd = generateLlcCommand(llcBin, llFile, objFile);
        std::string clangCmd = generateClangCommand(clangBin, objFile, exeFile);
        logger << "Executing commands:" << std::endl << "  " << llcCmd << std::endl << "  " << clangCmd << std::endl;
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
