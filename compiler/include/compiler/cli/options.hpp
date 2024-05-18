#pragma once

#include <exception>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#if defined(ENABLE_CODEGEN_AST_TO_LLVMIR) || defined(ENABLE_CODEGEN_OPTREE_TO_LLVMIR)
#define LLVMIR_CODEGEN_ENABLED
#endif

namespace cli {

namespace arg {

constexpr std::string_view help = "--help";
constexpr std::string_view version = "--version";
constexpr std::string_view debug = "--debug";
constexpr std::string_view optimize = "--optimize";
constexpr std::string_view time = "--time";
constexpr std::string_view stopAfter = "--stop-after";
constexpr std::string_view path = "--path";
constexpr std::string_view files = "FILES";

#ifdef LLVMIR_CODEGEN_ENABLED
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
constexpr std::string_view converter = "converter";
constexpr std::string_view semantizer = "semantizer";
constexpr std::string_view optimizer = "optimizer";

#ifdef LLVMIR_CODEGEN_ENABLED
constexpr std::string_view codegen = "codegen";
#endif

} // namespace stage

namespace compilation_path {

constexpr std::string_view ast = "ast";
constexpr std::string_view optree = "optree";

} // namespace compilation_path

struct Options {
    bool debug;
    std::string path;
    bool time;
    bool optimize;
    std::optional<std::string> stopAfter;
#ifdef LLVMIR_CODEGEN_ENABLED
    bool compile;
    std::string clang;
    std::string llc;
    std::string output;
#endif
    std::vector<std::string> files;
    std::string helpMessage;

    void dump() const;
};

class OptionsError : public std::exception {
    std::string message;

  public:
    OptionsError(const std::string &message) : message(message){};
    ~OptionsError() = default;

    OptionsError &operator=(const OptionsError &) noexcept = default;

    const char *what() const noexcept override {
        return message.c_str();
    }
};

Options parseArguments(int argc, const char *const argv[]);

} // namespace cli
