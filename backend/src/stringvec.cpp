#include "stringvec.hpp"

std::ostream &operator<<(std::ostream &out, const StringVec &vec) {
    for (auto &str : vec)
        out << str << '\n';
    return out;
}
