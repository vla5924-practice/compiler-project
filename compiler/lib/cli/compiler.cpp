#include "compiler.hpp"

#include <algorithm>
#include <chrono>
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
#include "compiler/utils/timer.hpp"

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

const char *const ARG_TIMES = "--times";
const char *const ARG_DISABLE_LOG = "--dissable-log";
const char *const ARG_LAST_MODULE = "--last-module";
const char *const ARG_FILES = "FILES";

#ifdef ENABLE_CODEGEN
const char *const ARG_COMPILE = "--compile";
const char *const ARG_CLANG = "--clang";
const char *const ARG_LLC = "--llc";
const char *const ARG_OUTPUT = "--output";
#endif

std::vector<std::string> possible_last_modules = {
    "preprocessor", "lexer",       "parser", "semantizer",
    "optimizer",    "ir_generator"}; // our version of argparser does not support "choises" parameter for arguments

argparse::ArgumentParser createArgumentParser() {
    argparse::ArgumentParser parser("compiler", "1.0", argparse::default_arguments::none);
    parser.add_argument("-h", ARG_HELP).help("show help message and exit").default_value(false).implicit_value(true);
    parser.add_argument("-v", ARG_VERBOSE).help("print info messages").default_value(false).implicit_value(true);
    parser.add_argument("-l", ARG_LOG).help("log file (stages output will be saved if provided)");
    parser.add_argument("-O", ARG_OPTIMIZE).help("run optimization pass").default_value(false).implicit_value(true);

    parser.add_argument("--times", ARG_TIMES)
        .help("measure executions times of modules")
        .default_value(false)
        .implicit_value(true);
    parser.add_argument("--dissable-log", ARG_DISABLE_LOG)
        .help("disable dump logging")
        .default_value(false)
        .implicit_value(true);
    parser.add_argument("--last-module", ARG_LAST_MODULE)
        .help("Breaks after module. ")
        .default_value(
#ifdef ENABLE_CODEGEN
            std::string("ir_generator")
#else
            std::string("optimizer")
#endif
        ); // .choises(possible_last_modules)

#ifdef ENABLE_CODEGEN
    parser.add_argument("-c", ARG_COMPILE)
        .help("produce an executable instead of LLVM IR code")
        .default_value(false)
        .implicit_value(true);
    parser.add_argument(ARG_CLANG)
        .help("path to clang executable (required if --compile argument is set)")
        .default_value("clang");
    parser.add_argument(ARG_LLC)
        .help("path to llc executable (required if --compile argument is set)")
        .default_value("llc");
    parser.add_argument("-o", ARG_OUTPUT).help("output file");
#endif
    parser.add_argument(ARG_FILES).help("source files (separated by spaces)").required().remaining();
    return parser;
}

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
    argparse::ArgumentParser program = createArgumentParser();
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

    std::string last_module = program.get<std::string>(ARG_LAST_MODULE);
    bool is_module_exists = possible_last_modules.cend() !=
                            std::find(possible_last_modules.cbegin(), possible_last_modules.cend(), last_module);
    if (!is_module_exists) {
        std::cerr << "Unexepted module name " << last_module << std::endl;
        std::cerr << "Exepted values: [ ";
        for (auto value : possible_last_modules) {
            std::cerr << value << ' ';
        }
        std::cerr << "]" << std::endl;
        return 2;
    }

    bool times = program.get<bool>(ARG_TIMES);
    bool disable_logging = program.get<bool>(ARG_DISABLE_LOG);
    ast::SyntaxTree tree;

    utils::Timer module_timer;

    std::vector<double> measured_times;

    try {
        logger << std::endl << "PREPROCESSOR:" << std::endl;
        module_timer.start();
        source = Preprocessor::process(source);
        module_timer.end();
        measured_times.push_back(module_timer.elapsed());
        if (times) {
            logger << "Elapsed time: " << std::to_string(measured_times.back()) << std::endl;
        }
        if (disable_logging) {
            logger << dumping::dump(source) << std::endl;
        }
        if (last_module == possible_last_modules[0]) {
            return 0;
        }

        logger << "LEXER:" << std::endl;
        module_timer.start();
        auto tokens = Lexer::process(source);
        module_timer.end();
        measured_times.push_back(module_timer.elapsed());
        if (times) {
            logger << "Elapsed time: " << std::to_string(measured_times.back()) << std::endl;
        }
        if (disable_logging) {
            logger << dumping::dump(tokens) << std::endl;
        }
        if (last_module == possible_last_modules[1]) {
            return 0;
        }

        logger << "PARSER:" << std::endl;
        module_timer.start();
        tree = Parser::process(tokens);
        module_timer.end();
        measured_times.push_back(module_timer.elapsed());
        if (times) {
            logger << "Elapsed time: " << std::to_string(measured_times.back()) << std::endl;
        }
        if (disable_logging) {
            logger << tree.dump();
        }
        if (last_module == possible_last_modules[2]) {
            return 0;
        }

        logger << "SEMANTIZER:" << std::endl;
        module_timer.start();
        Semantizer::process(tree);
        module_timer.end();
        measured_times.push_back(module_timer.elapsed());
        if (times) {
            logger << "Elapsed time: " << std::to_string(measured_times.back()) << std::endl;
        }
        if (disable_logging) {
            logger << tree.dump();
        }
        if (last_module == possible_last_modules[3]) {
            return 0;
        }

        if (program.get<bool>(ARG_OPTIMIZE)) {
            logger << "OPTIMIZER:" << std::endl;
            module_timer.start();
            Optimizer::process(tree);
            module_timer.end();
            measured_times.push_back(module_timer.elapsed());
            if (times) {
                logger << "Elapsed time: " << std::to_string(measured_times.back()) << std::endl;
            }
            if (disable_logging) {
                logger << tree.dump();
            }
            if (last_module == possible_last_modules[4]) {
                return 0;
            }
        }
        if (times) {
            logger << "Compile time: "
                   << std::to_string(std::accumulate(measured_times.begin(), measured_times.end(), 0.0)) << std::endl;
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
