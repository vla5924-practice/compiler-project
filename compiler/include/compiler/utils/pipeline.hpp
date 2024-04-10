#pragma once

#include <functional>
#include <type_traits>

namespace utils {

template <typename CurrentFn, typename NextFn, typename CurrentIn>
struct PipelineLink {
    using CurrentOut = std::invoke_result_t<CurrentFn, CurrentIn>;
    using NextIn = std::conditional_t<std::is_invocable_v<NextFn, CurrentOut &>, CurrentOut, const CurrentOut>;
    using NextOut = std::invoke_result_t<NextFn, NextIn>;
    static_assert(!std::is_same_v<CurrentOut, void>, "CurrentFn cannot have void as an invocation result type");

    std::function<CurrentOut(CurrentIn &)> current;
    std::function<NextOut(NextIn &)> next;

    PipelineLink() = delete;
    PipelineLink(const CurrentFn &current, const NextFn &next) : current(current), next(next){};
    PipelineLink(const PipelineLink &) = default;
    PipelineLink(PipelineLink &&) = default;
    ~PipelineLink() = default;

    NextOut operator()(CurrentIn &in) {
        CurrentOut out = current(in);
        if constexpr (std::is_same_v<NextOut, void>)
            next(out);
        else
            return next(out);
    }

    template <typename AppendFn>
    PipelineLink<PipelineLink, AppendFn, CurrentIn> operator>>(const AppendFn &append) {
        return {*this, append};
    }
};

template <typename In, typename Fn>
auto pipeline(const Fn &first) {
    auto thru = [](In &arg) -> In & { return arg; };
    return PipelineLink<decltype(thru), Fn, In>(thru, first);
}

} // namespace utils
