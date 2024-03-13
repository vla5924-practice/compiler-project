#pragma once

#include "compiler/utils/source_files.hpp"

namespace preprocessor {

class Preprocessor {
  public:
    Preprocessor() = delete;
    Preprocessor(const Preprocessor &) = delete;
    Preprocessor(Preprocessor &&) = delete;
    ~Preprocessor() = delete;

    static utils::SourceFile process(const utils::SourceFile &source);
};

} // namespace preprocessor
