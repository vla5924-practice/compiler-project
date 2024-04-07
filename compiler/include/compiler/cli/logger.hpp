#pragma once

#include <fstream>
#include <iostream>
#include <string>

class Logger {
  public:
    Logger();
    ~Logger();

    Logger(const Logger &) = delete;
    Logger(Logger &&) = delete;

    void setStdoutEnabled(bool enabled);
    void setOutputFile(const std::string &filename);

    template <typename T>
    Logger &operator<<(const T &out) {
        if (stdoutEnabled)
            std::cout << out;
        if (fileOutput.is_open())
            fileOutput << out;
        return *this;
    }

  private:
    void closeOutputFile();

    bool stdoutEnabled;
    std::ofstream fileOutput;
};
