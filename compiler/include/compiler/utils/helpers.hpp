#pragma once

#include <iterator>
#include <memory>
#include <ostream>
#include <type_traits>

namespace utils {

namespace detail {

template <typename Iterator>
struct AdvanceEarlyRange {
    struct AdvanceEarlyIterator {
        using ValueRef = std::remove_reference_t<decltype(std::declval<Iterator>().operator*())> &;

        Iterator it;

        AdvanceEarlyIterator(Iterator it) : it(it){};
        ~AdvanceEarlyIterator() = default;

        ValueRef operator*() {
            ValueRef value = *it;
            std::advance(it, 1);
            return value;
        }

        bool operator!=(const AdvanceEarlyIterator &other) const {
            return it != other.it;
        }

        AdvanceEarlyIterator &operator++() {
            return *this;
        }
    };

    Iterator beginIt;
    Iterator endIt;

    AdvanceEarlyRange(Iterator beginIt, Iterator endIt) : beginIt(beginIt), endIt(endIt){};
    ~AdvanceEarlyRange() = default;

    AdvanceEarlyIterator begin() {
        return AdvanceEarlyIterator(beginIt);
    }

    AdvanceEarlyIterator end() {
        return AdvanceEarlyIterator(endIt);
    }
};

} // namespace detail

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

template <typename Iterator>
auto advanceEarly(Iterator begin, Iterator end) {
    return detail::AdvanceEarlyRange<Iterator>(begin, end);
}

template <typename... Types, typename Type>
inline bool isAny(const std::shared_ptr<Type> &object) {
    return (object->template is<Types>() || ...);
}

template <typename... Types, typename Type>
inline bool isAny(const Type &object) {
    return (object.template is<Types>() || ...);
}

} // namespace utils
