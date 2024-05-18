#include "options.hpp"

#include <argparse/argparse.hpp>

#include "version.hpp"

namespace cli {

Options parseArguments(int argc, const char *const argv[]) {
    argparse::ArgumentParser program("compiler", version, argparse::default_arguments::none);
    program.add_argument("-h", arg::help).help("show help message and exit").flag();
    program.add_argument(arg::debug).help("print debug info").flag();
    program.add_argument(arg::path)
        .help("compilation path")
        .choices(compilation_path::ast, compilation_path::optree)
        .default_value(compilation_path::optree);
    program.add_argument(arg::time).help("print execution times of each stage").flag();
    program.add_argument(arg::stopAfter)
        .help("stop processing after specific stage")
        .choices(stage::preprocessor, stage::lexer, stage::parser, stage::converter, stage::semantizer, stage::optimizer
#ifdef LLVMIR_CODEGEN_ENABLED
                 ,
                 stage::codegen
#endif
        );
    program.add_argument("-O", arg::optimize).help("perform optimizations").nargs(argparse::nargs_pattern::optional);
#ifdef LLVMIR_CODEGEN_ENABLED
    program.add_argument("-c", arg::compile).help("produce an executable instead of LLVM IR code").flag();
    program.add_argument(arg::clang).help("path to clang executable").default_value("clang");
    program.add_argument(arg::llc).help("path to llc executable").default_value("llc");
    program.add_argument("-o", arg::output).help("output file");
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
    options.help = program.get<bool>(arg::help);
    options.debug = program.get<bool>(arg::debug);
    options.path = program.get<std::string>(arg::path);
    options.time = program.get<bool>(arg::time);
    if (program.is_used(arg::stopAfter))
        options.stopAfter = program.get<std::string>(arg::stopAfter);
    if (program.is_used(arg::optimize))
        options.optimize = program.get<std::string>(arg::optimize);
#ifdef LLVMIR_CODEGEN_ENABLED
    options.compile = program.get<bool>(arg::compile);
    options.clang = program.get<std::string>(arg::clang);
    options.llc = program.get<std::string>(arg::llc);
    options.output = program.get<std::string>(arg::output);
#endif
    options.files = program.get<std::vector<std::string>>(arg::files);
    options.helpMessage = program.help().str();
    return options;
}

} // namespace cli
