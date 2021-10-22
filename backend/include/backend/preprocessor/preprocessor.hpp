#pragma once

#include "stringvec.hpp"

namespace preprocessor {

class Preprocessor {
    static StringVec removeComments(const StringVec &source);

  public:
    Preprocessor() = delete;
    Preprocessor(const Preprocessor &) = delete;
    Preprocessor(Preprocessor &&) = delete;
    ~Preprocessor() = delete;

    static StringVec process(const StringVec &source);
};

} // namespace preprocessor