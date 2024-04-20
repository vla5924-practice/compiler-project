#include "optimizer/optimizer.hpp"

#include <cstddef>
#include <unordered_map>
#include <vector>

#include "compiler/optree/operation.hpp"
#include "compiler/optree/program.hpp"

#include "optimizer/opt_builder.hpp"
#include "optimizer/transform_factories.hpp"

using namespace optree;
using namespace optree::optimizer;

class OperationSet {
    std::vector<Operation::Ptr> data;
    std::unordered_map<const Operation *, size_t> positions;

  public:
    OperationSet() {
        constexpr size_t capacity = 64U;
        data.reserve(capacity);
    }

    OperationSet(const OperationSet &) = default;
    OperationSet(OperationSet &&) = default;
    ~OperationSet() = default;

    bool empty() const {
        return positions.empty();
    }

    void push(const Operation::Ptr &op) {
        if (positions.contains(op.get()))
            return;
        positions[op.get()] = data.size();
        data.emplace_back(op);
    }

    Operation::Ptr pop() {
        while (!data.back())
            data.pop_back();
        Operation::Ptr op = data.back();
        data.pop_back();
        positions.erase(op.get());
        while (!data.empty() && !data.back())
            data.pop_back();
        return op;
    }

    void erase(const Operation::Ptr &op) {
        auto it = positions.find(op.get());
        if (it == positions.end())
            return;
        data[it->second].reset();
        positions.erase(it);
    }

    void clear() {
        data.clear();
        positions.clear();
    }
};

namespace {

void pushToSet(const Operation::Ptr &root, OperationSet &ops) {
    for (const auto &op : root->body)
        pushToSet(op, ops);
    ops.push(root);
}

} // namespace

Optimizer::Optimizer() : iterLimit(100U) {
    transforms.emplace_back(createEraseUnusedOps());
}

void Optimizer::process(Program &program) const {
    OperationSet ops;
    bool mutated = false;
    OptBuilder::Notifier notifier;
    notifier.onInsert = [&ops, &mutated](const Operation::Ptr &op) {
        ops.push(op);
        mutated = true;
    };
    notifier.onUpdate = [&ops, &mutated](const Operation::Ptr &op) {
        ops.push(op);
        mutated = true;
    };
    notifier.onErase = [&ops, &mutated](const Operation::Ptr &op) {
        ops.erase(op);
        mutated = true;
    };
    size_t iter = 0;
    do {
        mutated = false;
        ops.clear();
        pushToSet(program.root, ops);
        while (!ops.empty()) {
            Operation::Ptr op = ops.pop();
            for (const auto &transform : transforms) {
                if (!transform->canRun(op))
                    continue;
                OptBuilder builder(notifier);
                builder.setInsertPointBefore(op);
                transform->run(op, builder);
            }
        }
    } while (mutated && ++iter < iterLimit);
}
