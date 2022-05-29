#include "logger.hpp"

#include <cassert>
#include <iostream>

#define OUTPUT_OPERATOR_IMPL(str)                                                                                      \
    if (stdoutEnabled)                                                                                                 \
        std::cout << (str);                                                                                            \
    if (fileOutput.is_open())                                                                                          \
        fileOutput << (str);                                                                                           \
    return *this

Logger::Logger() : stdoutEnabled(false) {
    assert(!fileOutput.is_open());
}

Logger::~Logger() {
    closeOutputFile();
}

void Logger::setStdoutEnabled(bool enabled) {
    stdoutEnabled = enabled;
}

void Logger::setOutputFile(const std::string &filename) {
    closeOutputFile();
    fileOutput.open(filename, std::ios_base::out);
}

void Logger::closeOutputFile() {
    if (fileOutput.is_open())
        fileOutput.close();
}

Logger &Logger::operator<<(const std::string &str) {
    OUTPUT_OPERATOR_IMPL(str);
}

Logger &Logger::operator<<(const char *const str) {
    OUTPUT_OPERATOR_IMPL(str);
}

Logger &Logger::operator<<(char chr) {
    OUTPUT_OPERATOR_IMPL(chr);
}

Logger &Logger::operator<<(size_t number) {
    OUTPUT_OPERATOR_IMPL(number);
}

Logger &Logger::operator<<(std::ostream &(*manip)(std::ostream &)) {
    OUTPUT_OPERATOR_IMPL(manip);
}
