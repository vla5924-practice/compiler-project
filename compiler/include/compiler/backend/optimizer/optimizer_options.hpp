#pragma once

#include <limits>

namespace optimizer {

class OptimizerOptions {
  public:
    using OptionsMask = unsigned int;

    enum Option : OptionsMask {
        Miscellaneous = 1 << 0,
        RemoveUnusedFunctions = 1 << 1,
        RemoveUnusedVariables = 1 << 2,
        All = std::numeric_limits<OptionsMask>::max(),
    };

    OptimizerOptions(OptionsMask options_ = 0) : options(options_){};

    OptimizerOptions(const OptimizerOptions &) = default;
    OptimizerOptions(OptimizerOptions &&) = default;
    ~OptimizerOptions() = default;

    bool has(Option option) const {
        return (options & option) == option;
    }

    void enable(Option option) {
        options |= option;
    }

    void disable(Option option) {
        options &= ~option;
    }

    void toggle(Option option) {
        options ^= option;
    }

    static OptimizerOptions all() {
        return OptimizerOptions(All);
    }

    static OptimizerOptions none() {
        return OptimizerOptions(0);
    }

    static OptimizerOptions except(Option option) {
        return OptimizerOptions(All & ~option);
    }

    static OptimizerOptions only(Option option) {
        return OptimizerOptions(option);
    }

    OptimizerOptions &operator=(const OptimizerOptions &) = default;
    OptimizerOptions &operator=(OptimizerOptions &&) = default;

    bool operator==(const OptimizerOptions &other) const {
        return options == other.options;
    }

    bool operator!=(const OptimizerOptions &other) const {
        return options != other.options;
    }

  private:
    OptionsMask options;
};

} // namespace optimizer
