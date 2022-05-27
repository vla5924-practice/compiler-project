#pragma once

#include <fstream>
#include <string>

class Logger {
  public:
    Logger();
    ~Logger();

    Logger(const Logger &) = delete;
    Logger(Logger &&) = delete;

    void setStdoutEnabled(bool enabled);
    void setOutputFile(const std::string &filename);

    Logger &operator<<(const std::string &);
    Logger &operator<<(const char *const);
    Logger &operator<<(char);
    Logger &operator<<(size_t);
    Logger &operator<<(std::ostream &(*)(std::ostream &));

  private:
    void closeOutputFile();

    bool stdoutEnabled;
    std::ofstream fileOutput;
};
