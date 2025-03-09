#include "helpers.hpp"

#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include "compiler/utils/platform.hpp"

#if defined(COMPILER_PLATFORM_WINDOWS)
#include <array>
#include <cstdio>
#include <string_view>
#elif defined(COMPILER_PLATFORM_LINUX)
#include <cstdlib>
#endif

namespace cli {

TemporaryDirectory::TemporaryDirectory() {
#if defined(COMPILER_PLATFORM_WINDOWS)
    std::array<char, L_tmpnam_s> tmpnamArg;
    tmpnam_s(tmpnamArg.data(), L_tmpnam_s);
    char *tmpnamResult = tmpnamArg.data();
    std::filesystem::create_directory(std::string_view(tmpnamArg.data()));
#elif defined(COMPILER_PLATFORM_LINUX)
    auto templatePath = std::filesystem::temp_directory_path() / "XXXXXX";
    std::string tmpnamArg(templatePath.string());
    mkstemp(tmpnamArg.data());
#endif
}

TemporaryDirectory::~TemporaryDirectory() {
    std::filesystem::remove_all(dir);
}

const std::filesystem::path &TemporaryDirectory::path() const {
    return dir;
}

std::string wrapQuotes(const std::string &str) {
    if (str.front() == '"' && str.back() == '"')
        return str;
    return "\"" + str + "\"";
}

std::string makeCommand(const std::vector<std::string> &args) {
    if (args.empty())
        return {};
    std::stringstream cmd;
    cmd << args.front();
    for (auto it = args.begin() + 1; it != args.end(); ++it)
        cmd << ' ' << wrapQuotes(*it);
    return cmd.str();
}

} // namespace cli
