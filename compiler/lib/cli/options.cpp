#include "options.hpp"

#include <iostream>

#include <argparse/argparse.hpp>

#include "version.hpp"

namespace cli {

void Options::dump() const {
    std::cerr << "debug=" << debug << ", path=" << path << ", time=" << time << ", optimize=" << optimize;
    if (stopAfter.has_value())
        std::cerr << ", stopAfter=" << stopAfter.value();
#ifdef LLVMIR_CODEGEN_ENABLED
    std::cerr << ", compile=" << compile << ", clang=" << clang << ", llc=" << llc << ", output=" << output;
#endif
    std::cerr << ", files=[ ";
    for (const auto &file : files)
        std::cerr << file << ' ';
    std::cerr << "]";
}

Options parseArguments(int argc, const char *const argv[]) {
    argparse::ArgumentParser program("compiler", version);
    program.add_argument(arg::debug).help("print debug info (stages output)").flag();
    program.add_argument(arg::path)
        .help("compilation path")
        .choices(compilation_path::ast, compilation_path::optree)
        .default_value(std::string(compilation_path::optree));
    program.add_argument(arg::time).help("print execution times of each stage").flag();
    program.add_argument(arg::stopAfter)
        .help("stop processing after specific stage")
        .choices(stage::preprocessor, stage::lexer, stage::parser, stage::converter, stage::semantizer, stage::optimizer
#ifdef LLVMIR_CODEGEN_ENABLED
                 ,
                 stage::codegen
#endif
        );
    program.add_argument("-O", arg::optimize).help("perform optimizations").flag();
#ifdef LLVMIR_CODEGEN_ENABLED
    program.add_argument("-c", arg::compile).help("produce an executable instead of LLVM IR code").flag();
    program.add_argument(arg::clang).help("path to clang executable").default_value("clang");
    program.add_argument(arg::llc).help("path to llc executable").default_value("llc");
    program.add_argument("-o", arg::output).help("output file").default_value("-");
#endif
    program.add_argument(arg::files)
        .help("source files (separated by spaces)")
        .required()
        .nargs(argparse::nargs_pattern::at_least_one);

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        throw OptionsError(err.what());
    }

    Options options;
    options.debug = program.get<bool>(arg::debug);
    options.path = program.get<std::string>(arg::path);
    options.time = program.get<bool>(arg::time);
    options.optimize = program.get<bool>(arg::optimize);
    if (program.is_used(arg::stopAfter))
        options.stopAfter = program.get<std::string>(arg::stopAfter);
#ifdef LLVMIR_CODEGEN_ENABLED
    options.compile = program.get<bool>(arg::compile);
    options.clang = program.get<std::string>(arg::clang);
    options.llc = program.get<std::string>(arg::llc);
    options.output = program.get<std::string>(arg::output);
#endif
    if (program.is_used(arg::files))
        options.files = program.get<std::vector<std::string>>(arg::files);
    options.helpMessage = program.help().str();
    return options;
}

} // namespace cli
