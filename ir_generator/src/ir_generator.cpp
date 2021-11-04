#include "ir_generator.hpp"

using namespace ir_generator;

IRGenerator::IRGenerator(const std::string &moduleName, bool emitDebugInfo)
    : module(new llvm::Module(llvm::StringRef(moduleName), context)), builder(new llvm::IRBuilder(context)) {
}
