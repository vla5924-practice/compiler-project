#include "helpers.hpp"

#include <array>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#ifdef COMPILER_PLATFORM_WINDOWS
#include <stdio.h>
#elifdef COMPILER_PLATFORM_LINUX
#include <cstdio>
#endif

#include "compiler/utils/platform.hpp"

namespace cli {

TemporaryDirectory::TemporaryDirectory() {
#ifdef COMPILER_PLATFORM_WINDOWS
    std::array<char, L_tmpnam_s> tmpnamArg;
    tmpnam_s(tmpnamArg.data(), L_tmpnam_s);
    char *tmpnamResult = tmpnamArg.data();
#elifdef COMPILER_PLATFORM_LINUX
    std::array<char, L_tmpnam> tmpnamArg;
    char *tmpnamResult = std::tmpnam(tmpnamArg.data());
#endif
    dir = tmpnamResult;
    std::filesystem::create_directory(dir);
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
