#include "compiler.hpp"
#include "compiler/backend/optree/optimizer/transform.hpp"

#include <exception>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

#ifdef LLVMIR_CODEGEN_ENABLED
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <ostream>
#endif

#include "compiler/backend/ast/optimizer/optimizer.hpp"
#include "compiler/backend/ast/semantizer/semantizer.hpp"
#include "compiler/backend/optree/optimizer/optimizer.hpp"
#include "compiler/backend/optree/optimizer/transform_factories.hpp"
#include "compiler/frontend/converter/converter.hpp"
#include "compiler/frontend/lexer/lexer.hpp"
#include "compiler/frontend/parser/parser.hpp"
#include "compiler/frontend/preprocessor/preprocessor.hpp"
#include "compiler/utils/debug.hpp"
#include "compiler/utils/error_buffer.hpp"
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
    std::vector<std::string> cmd = {
        llcBin,
#ifdef COMPILER_PLATFORM_LINUX
        "-relocation-model=pic",
#endif
        "-filetype=obj",
        llFile.string(),
        "-o",
        objFile.string(),
    };
    return makeCommand(cmd);
}

std::string objToExe(const std::string &clangBin, const std::filesystem::path &objFile,
                     const std::filesystem::path &exeFile) {
    std::vector<std::string> cmd = {
        clangBin,
#ifdef COMPILER_PLATFORM_LINUX
        "-fPIE",
#endif
        objFile.string(), "-o", exeFile.string(),
    };
    return makeCommand(cmd);
}

int runCommand(const std::string &cmd) {
    int ret = std::system(cmd.c_str());
    if (ret != 0) {
        std::cerr << "Process returned " << ret << ": " << cmd << '\n';
    }
    return ret;
}

int runLLVMIRGenerator(const Options &opt, const std::function<void(std::ostream &)> &dumpToStream,
                       const std::function<void(const std::string &)> &dumpToFile) {
    if (opt.debug) {
        std::cerr << "LLVMIR GENERATOR:\n";
        dumpToStream(std::cerr);
    }
    bool printOutput = opt.output == "-";
    if (!opt.compile) {
        if (printOutput)
            dumpToStream(std::cout);
        else
            dumpToFile(opt.output);
        return 0;
    }
    if (printOutput) {
        std::cerr << "Unable to print binary file to stdout. Please, provide --output argument.\n";
        return 3;
    }
    TemporaryDirectory tempDir;
    try {
        auto llFile = tempDir.path() / "out.ll";
        dumpToFile(llFile.string());
        auto objFile = tempDir.path() / "out.obj";
        auto exeFile = tempDir.path() / "out.exe";
        auto llcCmd = llToObj(opt.llc, llFile, objFile);
        auto clangCmd = objToExe(opt.clang, objFile, exeFile);
        if (opt.debug) {
            std::cerr << "Executing commands:\n"
                      << "  " << llcCmd << "\n"
                      << "  " << clangCmd << "\n";
        }
        bool cmdFailed = (runCommand(llcCmd) || runCommand(clangCmd));
        if (cmdFailed)
            return 3;
        std::filesystem::copy_file(exeFile, opt.output, std::filesystem::copy_options::overwrite_existing);
    } catch (std::exception &e) {
        std::cerr << e.what();
        return 3;
    }
    return 0;
}
#endif

} // namespace

Compiler::Compiler(const Options &options) : opt(options) {
    if (opt.time)
        measuredTimes.reserve(8);
}

int Compiler::readFiles() {
    if (opt.files.empty()) {
        std::cerr << "No files provided\n";
        return 2;
    }
    try {
        for (const std::string &path : opt.files) {
            if (!std::filesystem::exists(path)) {
                std::cerr << "File is non-existent: " << path << '\n';
                return 2;
            }
            SourceFile other = utils::readFile(std::filesystem::canonical(path).string());
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
    if (opt.debug)
        std::cerr << "PREPROCESSOR:\n" << dumping::dump(source);
    if (opt.time)
        measuredTimes.emplace_back(stage::preprocessor, timer.elapsed());
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
    if (opt.debug)
        std::cerr << "LEXER:\n" << dumping::dump(tokens);
    if (opt.time)
        measuredTimes.emplace_back(stage::lexer, timer.elapsed());
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
    if (opt.time)
        measuredTimes.emplace_back(stage::parser, timer.elapsed());
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
    if (opt.time)
        measuredTimes.emplace_back(stage::converter, timer.elapsed());
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
    if (opt.time)
        measuredTimes.emplace_back(stage::semantizer, timer.elapsed());
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
    if (opt.time)
        measuredTimes.emplace_back(stage::optimizer, timer.elapsed());
    return 0;
}

#ifdef ENABLE_CODEGEN_AST_TO_LLVMIR
int Compiler::runAstLLVMIRGenerator() {
    Timer timer;
    ir_generator::IRGenerator generator(opt.files.front());
    timer.start();
    generator.process(tree);
    timer.stop();
    if (auto ret = runLLVMIRGenerator(
            opt, [&g = generator](std::ostream &str) { str << g.dump(); },
            [&g = generator](const std::string &str) { g.writeToFile(str); })) {
        return ret;
    }
    if (opt.time)
        measuredTimes.emplace_back(stage::codegen, timer.elapsed());
    return 0;
}
#endif

int Compiler::runOptreeOptimizer() {
    using namespace optree::optimizer;
    Timer timer;
    try {
        Optimizer optimizer;
        auto canonicalizer = CascadeTransform::make("Canonicalizer");
        canonicalizer->add(createEraseUnusedOps());
        canonicalizer->add(createFoldConstants());
        optimizer.add(canonicalizer);
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
    if (opt.time)
        measuredTimes.emplace_back(stage::optimizer, timer.elapsed());
    return 0;
}

#ifdef ENABLE_CODEGEN_OPTREE_TO_LLVMIR
int Compiler::runOptreeLLVMIRGenerator() {
    Timer timer;
    optree::llvmir_generator::LLVMIRGenerator generator(opt.files.front());
    timer.start();
    generator.process(program);
    timer.stop();
    if (auto ret = runLLVMIRGenerator(
            opt, [&g = generator](std::ostream &str) { str << g.dump(); },
            [&g = generator](const std::string &str) { g.dumpToFile(str); })) {
        return ret;
    }
    if (opt.time)
        measuredTimes.emplace_back(stage::codegen, timer.elapsed());
    return 0;
}
#endif

int Compiler::run() {
    if (opt.debug) {
        std::cerr << "Provided options: ";
        opt.dump();
        std::cerr << '\n';
    }
    RETURN_IF_NONZERO(readFiles());
    RETURN_IF_NONZERO(runPreprocessor());
    RETURN_IF_STOPAFTER(opt, stage::preprocessor);
    RETURN_IF_NONZERO(runLexer());
    RETURN_IF_STOPAFTER(opt, stage::lexer);
    RETURN_IF_NONZERO(runParser());
    RETURN_IF_STOPAFTER(opt, stage::parser);
    if (opt.backend == backend::ast) {
        RETURN_IF_NONZERO(runAstSemantizer());
        RETURN_IF_STOPAFTER(opt, stage::semantizer);
        RETURN_IF_NONZERO(runAstOptimizer());
        RETURN_IF_STOPAFTER(opt, stage::optimizer);
#ifdef ENABLE_CODEGEN_AST_TO_LLVMIR
        if (opt.codegen == codegen::llvm) {
            RETURN_IF_NONZERO(runAstLLVMIRGenerator());
            RETURN_IF_STOPAFTER(opt, stage::codegen);
        } else {
            COMPILER_UNREACHABLE("unexpected codegen");
        }
#endif
    } else if (opt.backend == backend::optree) {
        RETURN_IF_NONZERO(runConverter());
        RETURN_IF_STOPAFTER(opt, stage::converter);
        if (opt.optimize) {
            RETURN_IF_NONZERO(runOptreeOptimizer());
            RETURN_IF_STOPAFTER(opt, stage::optimizer);
        }
#ifdef ENABLE_CODEGEN_OPTREE_TO_LLVMIR
        if (opt.codegen == codegen::llvm) {
            RETURN_IF_NONZERO(runOptreeLLVMIRGenerator());
            RETURN_IF_STOPAFTER(opt, stage::codegen);
        } else {
            COMPILER_UNREACHABLE("unexpected codegen");
        }
#endif
    } else {
        COMPILER_UNREACHABLE("unexpected backend");
    }
    return 0;
}
