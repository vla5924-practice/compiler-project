#include "compiler.hpp"

#include <exception>
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#ifdef ENABLE_CODEGEN_AST_TO_LLVMIR
#include <cstdlib>
#include <filesystem>
#include <random>
#include <sstream>
#endif

#include "compiler/backend/ast/optimizer/optimizer.hpp"
#include "compiler/backend/ast/semantizer/semantizer.hpp"
#include "compiler/backend/optree/optimizer/optimizer.hpp"
#include "compiler/backend/optree/optimizer/transform_factories.hpp"
#include "compiler/frontend/converter/converter.hpp"
#include "compiler/frontend/lexer/lexer.hpp"
#include "compiler/frontend/parser/parser.hpp"
#include "compiler/frontend/preprocessor/preprocessor.hpp"
#include "compiler/utils/source_files.hpp"
#include "compiler/utils/timer.hpp"

#ifdef ENABLE_CODEGEN_AST_TO_LLVMIR
#include "compiler/codegen/ast_to_llvmir/ir_generator.hpp"
#endif
#ifdef ENABLE_CODEGEN_OPTREE_TO_LLVMIR
#include "compiler/codegen/optree_to_llvmir/llvmir_generator.hpp"
#endif

#include "dumping.hpp"
#include "helpers.hpp"
#include "options.hpp"

#define RETURN_IF_NONZERO(EXPR)                                                                                        \
    do {                                                                                                               \
        if (auto ret = (EXPR))                                                                                         \
            return ret;                                                                                                \
    } while (0)

#define RETURN_IF_STOPAFTER(OPTIONS, STAGE)                                                                            \
    do {                                                                                                               \
        if ((OPTIONS).stopAfter == (STAGE))                                                                            \
            return 0;                                                                                                  \
    } while (0)

using utils::SourceFile;
using utils::Timer;

using namespace cli;

namespace {

#ifdef LLVMIR_CODEGEN_ENABLED
std::string llToObj(const std::string &llcBin, const std::filesystem::path &llFile,
                    const std::filesystem::path &objFile) {
    std::vector<std::string> cmd = {llcBin, "-filetype=obj", llFile.string(), "-o", objFile.string()};
    return makeCommand(cmd);
}

std::string objToExe(const std::string &clangBin, const std::filesystem::path &objFile,
                     const std::filesystem::path &exeFile) {
    std::vector<std::string> cmd = {clangBin, objFile.string(), "-o", exeFile.string()};
    return makeCommand(cmd);
}
#endif

} // namespace

Compiler::Compiler(const Options &options) : opt(options) {
}

int Compiler::readFiles() {
    if (opt.files.empty()) {
        std::cerr << "No files provided\n";
        return 2;
    }
    if (opt.debug) {
        std::cerr << opt.files.size() << " file(s) provided:\n";
        for (const auto &file : opt.files)
            std::cerr << "  " << file << "\n";
    }
    try {
        for (const std::string &path : opt.files) {
            SourceFile other = utils::readFile(path);
            source.insert(source.end(), std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()));
            if (opt.debug)
                std::cerr << "Read file " << path << "\n";
        }
    } catch (std::exception &e) {
        std::cerr << e.what();
        return 3;
    }
    return 0;
}

int Compiler::runPreprocessor() {
    Timer timer;
    try {
        timer.start();
        source = preprocessor::Preprocessor::process(source);
        timer.stop();
    } catch (const ErrorBuffer &errors) {
        std::cerr << errors.message();
        return 3;
    }
    if (opt.debug) {
        std::cerr << "PREPROCESSOR:\n";
        dumping::dump(source);
    }
    measuredTimes[stage::preprocessor] = timer.elapsed();
    return 0;
}

int Compiler::runLexer() {
    Timer timer;
    try {
        timer.start();
        tokens = lexer::Lexer::process(source);
        timer.stop();
    } catch (const ErrorBuffer &errors) {
        std::cerr << errors.message();
        return 3;
    }
    if (opt.debug) {
        std::cerr << "LEXER:\n";
        dumping::dump(tokens);
    }
    measuredTimes[stage::lexer] = timer.elapsed();
    return 0;
}

int Compiler::runParser() {
    Timer timer;
    try {
        timer.start();
        tree = parser::Parser::process(tokens);
        timer.stop();
    } catch (const ErrorBuffer &errors) {
        std::cerr << errors.message();
        return 3;
    }
    if (opt.debug) {
        std::cerr << "PARSER:\n";
        tree.dump(std::cerr);
    }
    tokens.clear();
    measuredTimes[stage::parser] = timer.elapsed();
    return 0;
}

int Compiler::runConverter() {
    Timer timer;
    try {
        timer.start();
        program.root = converter::Converter::process(tree).root;
        timer.stop();
    } catch (const ErrorBuffer &errors) {
        std::cerr << errors.message();
        return 3;
    }
    if (opt.debug) {
        std::cerr << "CONVERTER:\n";
        program.root->dump(std::cerr);
    }
    measuredTimes[stage::converter] = timer.elapsed();
    return 0;
}

int Compiler::runAstSemantizer() {
    Timer timer;
    try {
        timer.start();
        semantizer::Semantizer::process(tree);
        timer.stop();
    } catch (const ErrorBuffer &errors) {
        std::cerr << errors.message();
        return 3;
    }
    if (opt.debug) {
        std::cerr << "SEMANTIZER:\n";
        tree.dump(std::cerr);
    }
    measuredTimes[stage::semantizer] = timer.elapsed();
    return 0;
}

int Compiler::runAstOptimizer() {
    Timer timer;
    try {
        timer.start();
        optimizer::Optimizer::process(tree);
        timer.stop();
    } catch (const ErrorBuffer &errors) {
        std::cerr << errors.message();
        return 3;
    }
    if (opt.debug) {
        std::cerr << "OPTIMIZER:\n";
        tree.dump(std::cerr);
    }
    measuredTimes[stage::optimizer] = timer.elapsed();
    return 0;
}

#ifdef ENABLE_CODEGEN_AST_TO_LLVMIR
int Compiler::runAstLLVMIRGenerator() {
    Timer timer;
    ir_generator::IRGenerator generator(opt.files.front());
    timer.start();
    generator.process(tree);
    timer.stop();

    if (opt.debug) {
        std::cerr << "LLVMIR GENERATOR:\n";
        std::cerr << generator.dump();
    }
    if (!opt.compile) {
        generator.writeToFile(opt.output);
        return 0;
    }

    TemporaryDirectory tempDir;
    try {
        auto llFile = tempDir.path() / "out.ll";
        generator.writeToFile(llFile.string());
        auto objFile = tempDir.path() / "out.obj";
        auto exeFile = tempDir.path() / "out.exe";
        auto llcCmd = llToObj(opt.llc, llFile, objFile);
        auto clangCmd = objToExe(opt.clang, objFile, exeFile);
        if (opt.debug) {
            std::cerr << "Executing commands:\n"
                      << "  " << llcCmd << "\n"
                      << "  " << clangCmd << "\n";
        }
        bool cmdFailed = (std::system(llcCmd.c_str()) || std::system(clangCmd.c_str()));
        if (cmdFailed)
            return 3;
        else
            std::filesystem::copy_file(exeFile, opt.output);
    } catch (std::exception &e) {
        std::cerr << e.what();
        return 3;
    }
    measuredTimes[stage::codegen] = timer.elapsed();
    return 0;
}
#endif

int Compiler::runOptreeOptimizer() {
    using namespace optree::optimizer;
    Timer timer;
    try {
        Optimizer optimizer;
        optimizer.add(createEraseUnusedOps());
        optimizer.add(createFoldConstants());
        optimizer.add(createEraseUnusedFunctions());
        timer.start();
        optimizer.process(program);
        timer.stop();
    } catch (const ErrorBuffer &errors) {
        std::cerr << errors.message();
        return 3;
    }
    if (opt.debug) {
        std::cerr << "OPTIMIZER:\n";
        program.root->dump(std::cerr);
    }
    measuredTimes[stage::optimizer] = timer.elapsed();
    return 0;
}

#ifdef ENABLE_CODEGEN_OPTREE_TO_LLVMIR
int Compiler::runOptreeLLVMIRGenerator() {
    Timer timer;
    optree::llvmir_generator::LLVMIRGenerator generator(opt.files.front());
    timer.start();
    generator.process(program);
    timer.stop();

    if (opt.debug) {
        std::cerr << "LLVMIR GENERATOR:\n";
        std::cerr << generator.dump();
    }
    if (!opt.compile) {
        generator.dumpToFile(opt.output);
        return 0;
    }

    TemporaryDirectory tempDir;
    try {
        auto llFile = tempDir.path() / "out.ll";
        generator.dumpToFile(llFile.string());
        auto objFile = tempDir.path() / "out.obj";
        auto exeFile = tempDir.path() / "out.exe";
        auto llcCmd = llToObj(opt.llc, llFile, objFile);
        auto clangCmd = objToExe(opt.clang, objFile, exeFile);
        if (opt.debug) {
            std::cerr << "Executing commands:\n"
                      << "  " << llcCmd << "\n"
                      << "  " << clangCmd << "\n";
        }
        bool cmdFailed = (std::system(llcCmd.c_str()) || std::system(clangCmd.c_str()));
        if (cmdFailed)
            return 3;
        else
            std::filesystem::copy_file(exeFile, opt.output);
    } catch (std::exception &e) {
        std::cerr << e.what();
        return 3;
    }
    measuredTimes[stage::codegen] = timer.elapsed();
    return 0;
}
#endif

int Compiler::run() {
    RETURN_IF_NONZERO(readFiles());
    RETURN_IF_NONZERO(runPreprocessor());
    RETURN_IF_STOPAFTER(opt, stage::preprocessor);
    RETURN_IF_NONZERO(runLexer());
    RETURN_IF_STOPAFTER(opt, stage::lexer);
    RETURN_IF_NONZERO(runParser());
    RETURN_IF_STOPAFTER(opt, stage::parser);
    if (opt.path == compilation_path::ast) {
        RETURN_IF_NONZERO(runAstSemantizer());
        RETURN_IF_STOPAFTER(opt, stage::semantizer);
        RETURN_IF_NONZERO(runAstOptimizer());
        RETURN_IF_STOPAFTER(opt, stage::optimizer);
#ifdef ENABLE_CODEGEN_AST_TO_LLVMIR
        RETURN_IF_NONZERO(runAstLLVMIRGenerator());
        RETURN_IF_STOPAFTER(opt, stage::codegen);
#endif
    } else if (opt.path == compilation_path::optree) {
        RETURN_IF_NONZERO(runConverter());
        RETURN_IF_STOPAFTER(opt, stage::converter);
        RETURN_IF_NONZERO(runOptreeOptimizer());
        RETURN_IF_STOPAFTER(opt, stage::optimizer);
#ifdef ENABLE_CODEGEN_OPTREE_TO_LLVMIR
        RETURN_IF_NONZERO(runOptreeLLVMIRGenerator());
        RETURN_IF_STOPAFTER(opt, stage::codegen);
#endif
    } else {
        std::cerr << "Unknown compilation path: " << opt.path << '\n';
        return 127;
    }
    return 0;
}
