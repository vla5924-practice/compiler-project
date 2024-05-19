#include "helpers.hpp"

#include <array>
#include <cstdio>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

namespace cli {

TemporaryDirectory::TemporaryDirectory() {
#if defined(_MSC_VER)
    std::array<char, L_tmpnam_s> tmpnamArg;
    tmpnam_s(tmpnamArg.data(), L_tmpnam_s);
    char *tmpnamResult = tmpnamArg.data();
#else
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
    std::stringstream cmd;
    for (const auto &arg : args)
        cmd << wrapQuotes(arg) << ' ';
    return cmd.str().c_str();
}

} // namespace cli
