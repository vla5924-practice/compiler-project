#pragma once

#include <string_view>
#include <utility>
#include <vector>

#include "compiler/ast/syntax_tree.hpp"
#include "compiler/frontend/lexer/token.hpp"
#include "compiler/optree/program.hpp"
#include "compiler/utils/source_files.hpp"

#include "compiler/cli/options.hpp"

namespace cli {

class Compiler {
    const Options &opt;
    std::vector<std::pair<std::string_view, long long>> measuredTimes;

    utils::SourceFile source;
    lexer::TokenList tokens;
    ast::SyntaxTree tree;
    optree::Program program;

    int readFiles();
    int runPreprocessor();
    int runLexer();
    int runParser();
    int runConverter();

    int runAstSemantizer();
    int runAstOptimizer();
#ifdef ENABLE_CODEGEN_AST_TO_LLVMIR
    int runAstLLVMIRGenerator();
#endif

    int runOptreeOptimizer();
#ifdef ENABLE_CODEGEN_OPTREE_TO_LLVMIR
    int runOptreeLLVMIRGenerator();
#endif

  public:
    Compiler(const Options &options);
    Compiler(const Compiler &) = delete;
    Compiler(Compiler &&) = delete;
    ~Compiler() = default;

    const auto &measurements() const {
        return measuredTimes;
    }

    int run();
};

} // namespace cli
