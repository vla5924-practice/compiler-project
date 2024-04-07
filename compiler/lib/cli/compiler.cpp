#include "compiler.hpp"

#include <exception>
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>
#include <string_view>
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
using utils::Timer;

#ifdef ENABLE_CODEGEN
using ir_generator::IRGenerator;
#endif

namespace arg {

constexpr std::string_view help = "--help";
constexpr std::string_view verbose = "--verbose";
constexpr std::string_view log = "--log";
constexpr std::string_view optimize = "--optimize";
constexpr std::string_view time = "--time";
constexpr std::string_view stopAfter = "--stop-after";
constexpr std::string_view files = "FILES";

#ifdef ENABLE_CODEGEN
constexpr std::string_view compile = "--compile";
constexpr std::string_view clang = "--clang";
constexpr std::string_view llc = "--llc";
constexpr std::string_view output = "--output";
#endif

} // namespace arg

namespace stage {

constexpr std::string_view preprocessor = "preprocessor";
constexpr std::string_view lexer = "lexer";
constexpr std::string_view parser = "parser";
constexpr std::string_view semantizer = "semantizer";
constexpr std::string_view optimizer = "optimizer";

#ifdef ENABLE_CODEGEN
constexpr std::string_view codegen = "codegen";
#endif

} // namespace stage

namespace {

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
    program.add_argument("-h", arg::help).help("show help message and exit").default_value(false).implicit_value(true);
    program.add_argument("-v", arg::verbose).help("print info messages").default_value(false).implicit_value(true);
    program.add_argument("-l", arg::log).help("log file (stages output will be saved if provided)");
    program.add_argument("-O", arg::optimize).help("run optimization pass").default_value(false).implicit_value(true);
#ifdef ENABLE_CODEGEN
    program.add_argument("-c", arg::compile)
        .help("produce an executable instead of LLVM IR code")
        .default_value(false)
        .implicit_value(true);
    program.add_argument(arg::clang)
        .help("path to clang executable (required if --compile argument is set)")
        .default_value("clang");
    program.add_argument(arg::llc)
        .help("path to llc executable (required if --compile argument is set)")
        .default_value("llc");
    program.add_argument("-o", arg::output).help("output file");
#endif
    program.add_argument(arg::time)
        .help("print execution times of each stage")
        .default_value(false)
        .implicit_value(true);
    program.add_argument(arg::stopAfter)
        .help("stop processing after specific stage")
        .choices(stage::preprocessor, stage::lexer, stage::parser, stage::semantizer, stage::optimizer);
    program.add_argument(arg::files)
        .help("source files (separated by spaces)")
        .required()
        .nargs(argparse::nargs_pattern::at_least_one);

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        std::cerr << err.what() << "\n";
        std::cerr << program;
        return 1;
    }

    if (program.get<bool>(arg::help)) {
        std::cout << program;
        return 0;
    }

    Logger logger;
    bool verbose = program.get<bool>(arg::verbose);
    if (verbose)
        logger.setStdoutEnabled(true);
    if (program.is_used(arg::log))
        logger.setOutputFile(program.get<std::string>(arg::log));

    std::vector<std::string> files;
    try {
        files = program.get<std::vector<std::string>>(arg::files);
        logger << files.size() << " file(s) provided:\n";
        for (auto &file : files)
            logger << "  " << file << "\n";
    } catch (std::logic_error &e) {
        std::cerr << "No files provided\n";
        return 2;
    }

    SourceFile source;
    try {
        for (const std::string &path : files) {
            SourceFile other = utils::readFile(path);
            source.insert(source.end(), std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()));
            logger << "Read file " << path << "\n";
        }
    } catch (std::exception &e) {
        logger << e.what();
        return 3;
    }

    std::string stopAfter;
    if (program.is_used(arg::stopAfter))
        stopAfter = program.get<std::string>(arg::stopAfter);
    bool times = program.get<bool>(arg::time);
    Timer timer;
    std::vector<long long> measuredTimes;

    ast::SyntaxTree tree;

    try {
        logger << "\nPREPROCESSOR:\n";
        timer.start();
        source = Preprocessor::process(source);
        timer.stop();
        measuredTimes.push_back(timer.elapsed());
        if (times)
            logger << "PREPROCESSOR Elapsed time: " << measuredTimes.back() << "\n";
        logger << dumping::dump(source) << "\n";
        if (stopAfter == stage::preprocessor)
            return 0;

        logger << "\nLEXER:\n";
        timer.start();
        auto tokens = Lexer::process(source);
        timer.stop();
        measuredTimes.push_back(timer.elapsed());
        if (times)
            logger << "LEXER Elapsed time: " << measuredTimes.back() << "\n";
        logger << dumping::dump(tokens) << "\n";
        if (stopAfter == stage::lexer)
            return 0;

        logger << "PARSER:\n";
        timer.start();
        tree = Parser::process(tokens);
        timer.stop();
        measuredTimes.push_back(timer.elapsed());
        if (times)
            logger << "PARSER Elapsed time: " << measuredTimes.back() << "\n";
        logger << tree.dump();
        if (stopAfter == stage::parser)
            return 0;

        logger << "SEMANTIZER:\n";
        timer.start();
        Semantizer::process(tree);
        timer.stop();
        measuredTimes.push_back(timer.elapsed());
        if (times)
            logger << "SEMANTIZER Elapsed time: " << measuredTimes.back() << "\n";
        logger << tree.dump();
        if (stopAfter == stage::semantizer)
            return 0;

        if (program.get<bool>(arg::optimize)) {
            logger << "OPTIMIZER:\n";
            timer.start();
            Optimizer::process(tree);
            timer.stop();
            measuredTimes.push_back(timer.elapsed());
            if (times)
                logger << "OPTIMIZER Elapsed time: " << measuredTimes.back() << "\n";
            logger << tree.dump();
        }

        if (stopAfter == stage::optimizer)
            return 0;
        if (times)
            logger << "Compile time: " << std::accumulate(measuredTimes.begin(), measuredTimes.end(), 0LL) << "\n";
    } catch (ErrorBuffer &errors) {
        std::cerr << errors.message();
        return 3;
    }

#ifdef ENABLE_CODEGEN
    logger << "\nIR GENERATOR:\n";
    IRGenerator generator("module");
    generator.process(tree);
    if (verbose) {
        generator.dump();
    }

    if (!program.is_used(arg::output))
        return 0;

    bool compile = program.get<bool>(arg::compile);
    std::string output = program.get<std::string>(arg::output);

    if (!compile) {
        generator.writeToFile(output);
        return 0;
    }

    std::string llcBin = program.is_used(arg::llc) ? program.get<std::string>(arg::llc) : "llc";
    std::string clangBin = program.is_used(arg::clang) ? program.get<std::string>(arg::clang) : "clang";

    try {
        std::filesystem::path tempDir = createTemporaryDirectory();
        const auto llFile = tempDir / "out.ll";
        generator.writeToFile(llFile.string());
        const auto objFile = tempDir / "out.obj";
        const auto exeFile = tempDir / "out.exe";
        std::string llcCmd = generateLlcCommand(llcBin, llFile, objFile);
        std::string clangCmd = generateClangCommand(clangBin, objFile, exeFile);
        logger << "Executing commands:\n"
               << "  " << llcCmd << "\n"
               << "  " << clangCmd << "\n";
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
    if (stopAfter == stage::codegen)
        return 0;
#endif

    return 0;
}
