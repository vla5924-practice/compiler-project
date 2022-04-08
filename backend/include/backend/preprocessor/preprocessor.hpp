#pragma once

#include <utils/source.hpp>
#include "stringvec.hpp"

namespace preprocessor {

class Preprocessor {
  public:
    Preprocessor() = delete;
    Preprocessor(const Preprocessor &) = delete;
    Preprocessor(Preprocessor &&) = delete;
    ~Preprocessor() = delete;

    static void process(utils::Source &source);
    static StringVec process(const StringVec &source);
};

} // namespace preprocessor
