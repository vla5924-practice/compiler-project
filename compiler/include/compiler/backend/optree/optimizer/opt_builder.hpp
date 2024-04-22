#pragma once

#include <functional>

#include "compiler/optree/builder.hpp"
#include "compiler/optree/operation.hpp"

namespace optree {
namespace optimizer {

class OptBuilder : public Builder {
  public:
    struct Notifier {
        using Callback = std::function<void(Operation *)>;

        Callback onInsert;
        Callback onUpdate;
        Callback onErase;
    };

    OptBuilder(const Notifier &notifier) : Builder(), notifier(notifier){};
    OptBuilder(const OptBuilder &) = delete;
    OptBuilder(OptBuilder &&) = default;
    ~OptBuilder() override = default;

    void insert(Operation *op) override;
    Operation *clone(Operation *op);
    void erase(Operation *op);
    void update(Operation *op, const std::function<void()> &actor);

  private:
    const Notifier &notifier;
};

} // namespace optimizer
} // namespace optree
