#include "logger.hpp"

#include <cassert>
#include <iostream>

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
