#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace cli {

class TemporaryDirectory {
    std::filesystem::path dir;

  public:
    TemporaryDirectory();
    TemporaryDirectory(const TemporaryDirectory &) = delete;
    TemporaryDirectory(TemporaryDirectory &&) = delete;
    ~TemporaryDirectory();

    const std::filesystem::path &path() const;
};

std::string wrapQuotes(const std::string &str);

std::string makeCommand(const std::vector<std::string> &args);

} // namespace cli
