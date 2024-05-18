#include "helpers.hpp"

#include <filesystem>
#include <sstream>
#include <stdio.h>

namespace cli {

TemporaryDirectory::TemporaryDirectory() {
    char tmpnamResult[L_tmpnam] = {0};
#if defined(_MSC_VER)
    tmpnam_s(tmpnamResult);
#else
    tmpnam(tmpnamResult);
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
