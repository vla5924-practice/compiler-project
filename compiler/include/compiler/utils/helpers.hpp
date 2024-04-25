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

template <typename Range>
class ReversedRange {
    Range &&range;

  public:
    ReversedRange(Range &&range) : range(range){};
    ~ReversedRange() = default;

    auto begin() const {
        return std::rbegin(range);
    }

    auto end() const {
        return std::rend(range);
    }
};

template <typename... Ranges>
class ZippedRanges {
    template <typename Range>
    using NativeIterator = decltype(std::begin(std::declval<Range>()));
    template <typename Range>
    using NativeValueType = decltype(*std::declval<NativeIterator<Range>>());

    std::tuple<Ranges &&...> ranges;

    class Iterator {
        std::tuple<NativeIterator<Ranges>...> iterators;

      public:
        Iterator() = delete;
        Iterator(const Iterator &) = default;
        Iterator(Iterator &&) = default;
        ~Iterator() = default;

        Iterator(NativeIterator<Ranges> &&...iterators) : iterators(iterators...){};

        Iterator &operator++() {
            std::apply([](auto &&...args) { ((++args), ...); }, iterators);
            return *this;
        }

        Iterator operator++(int) {
            auto temp = *this;
            ++*this;
            return temp;
        }

        bool operator==(const Iterator &other) const {
            return iterators == other.iterators;
        }

        bool operator!=(const Iterator &other) const {
            return !(*this == other);
        }

        auto operator*() {
            return std::apply([](auto &&...args) { return std::make_tuple((*args)...); }, iterators);
        }
    };

  public:
    ZippedRanges() = delete;
    ZippedRanges(const ZippedRanges &) = delete;
    ZippedRanges(ZippedRanges &&) = default;
    ~ZippedRanges() = default;

    ZippedRanges(Ranges &&...ranges) : ranges(ranges...){};

    Iterator begin() const {
        return std::apply([](auto &&...args) { return Iterator(std::begin(args)...); }, ranges);
    }

    Iterator end() const {
        return std::apply([](auto &&...args) { return Iterator(std::end(args)...); }, ranges);
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

template <typename Range>
auto reversed(Range &&range) {
    return detail::ReversedRange<Range>(range);
}

template <typename... Ranges>
auto zip(Ranges &&...ranges) {
    return detail::ZippedRanges<Ranges...>(ranges...);
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
