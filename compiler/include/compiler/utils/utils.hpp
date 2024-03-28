#pragma once

#include <iterator>
#include <ostream>

namespace utils {

template <typename Range, typename UnaryPred, typename NullaryPred>
void interleave(const Range &values, const UnaryPred &printValue, const NullaryPred &printSep) {
    if (std::empty(values))
        return;
    auto first = std::begin(values);
    printValue(*first);
    for (auto it = std::next(first); it != std::end(values); ++it) {
        printSep();
        printValue(*it);
    }
}

template <typename Range, typename UnaryPred>
void interleaveComma(std::ostream &stream, const Range &values, const UnaryPred &printValue) {
    interleave(values, printValue, [&stream] { stream << ", "; });
}

} // namespace utils
