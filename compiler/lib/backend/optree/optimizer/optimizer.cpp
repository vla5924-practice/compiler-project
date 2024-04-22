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
    std::vector<Operation *> data;
    std::unordered_map<Operation *, size_t> positions;

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

    void push(Operation *op) {
        if (positions.contains(op))
            return;
        positions[op] = data.size();
        data.emplace_back(op);
    }

    Operation *pop() {
        while (!data.back())
            data.pop_back();
        Operation *op = data.back();
        data.pop_back();
        positions.erase(op);
        while (!data.empty() && !data.back())
            data.pop_back();
        return op;
    }

    void erase(Operation *op) {
        auto it = positions.find(op);
        if (it == positions.end())
            return;
        data[it->second] = nullptr;
        positions.erase(it);
    }

    void clear() {
        data.clear();
        positions.clear();
    }
};

namespace {

void pushToSet(Operation *root, OperationSet &ops) {
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
    notifier.onInsert = [&ops, &mutated](Operation *op) {
        ops.push(op);
        mutated = true;
    };
    notifier.onUpdate = [&ops, &mutated](Operation *op) {
        ops.push(op);
        mutated = true;
    };
    notifier.onErase = [&ops, &mutated](Operation *op) {
        ops.erase(op);
        mutated = true;
    };
    size_t iter = 0;
    do {
        mutated = false;
        ops.clear();
        pushToSet(program.root, ops);
        while (!ops.empty()) {
            Operation *op = ops.pop();
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
