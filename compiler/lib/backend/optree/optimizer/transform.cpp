#include "optimizer/transform.hpp"

#include <cstddef>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "compiler/optree/operation.hpp"
#include "compiler/utils/debug.hpp"

#include "optimizer/opt_builder.hpp"

using namespace optree;
using namespace optree::optimizer;

using dbg = utils::DebugPrinter;

namespace {

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

class MutationTracker {
    Operation *const trackedOp;
    bool updatedTag;
    bool erasedTag;

  public:
    MutationTracker(const MutationTracker &) = delete;
    MutationTracker(MutationTracker &&) = delete;
    ~MutationTracker() = default;

    explicit MutationTracker(const Operation::Ptr &trackedOp)
        : trackedOp(trackedOp.get()), updatedTag(false), erasedTag(false){};

    bool updated() const {
        return updatedTag;
    }
    bool erased() const {
        return erasedTag;
    }

    void raiseUpdated(const Operation::Ptr &op) {
        if (op.get() == trackedOp)
            updatedTag = true;
    }
    void raiseErased(const Operation::Ptr &op) {
        if (op.get() == trackedOp)
            erasedTag = true;
    }
};

void pushToSet(const Operation::Ptr &root, OperationSet &ops) {
    for (const auto &op : root->body)
        pushToSet(op, ops);
    ops.push(root);
}

OptBuilder::Notifier makeNotifier(OperationSet &ops, bool &mutated, MutationTracker &tracker) {
    OptBuilder::Notifier notifier;
    notifier.onInsert = [&ops, &mutated](const Operation::Ptr &op) {
        ops.push(op);
        mutated = true;
    };
    notifier.onUpdate = [&ops, &mutated, &tracker](const Operation::Ptr &op) {
        ops.push(op);
        mutated = true;
        tracker.raiseUpdated(op);
    };
    notifier.onErase = [&ops, &mutated, &tracker](const Operation::Ptr &op) {
        ops.erase(op);
        mutated = true;
        tracker.raiseErased(op);
    };
    return notifier;
}

} // namespace

bool BaseTransform::recurse() const {
    return true;
}

CascadeTransform::CascadeTransform(std::string_view commonName, size_t iterLimit)
    : commonName(commonName), iterLimit(iterLimit) {
}

std::string_view CascadeTransform::name() const {
    return commonName;
}

bool CascadeTransform::canRun([[maybe_unused]] const Operation::Ptr &op) const {
    return true;
}

void CascadeTransform::run(const Operation::Ptr &op, [[maybe_unused]] OptBuilder &builder) const {
    OperationSet ops;
    bool mutated = false;
    size_t iter = 0;
    do {
        mutated = false;
        ops.clear();
        pushToSet(op, ops);
        while (!ops.empty()) {
            Operation::Ptr op = ops.pop();
            MutationTracker tracker(op);
            auto notifier = makeNotifier(ops, mutated, tracker);
            for (const auto &transform : transforms) {
                if (tracker.erased())
                    break;
                if (!transform->canRun(op))
                    continue;
                OptBuilder builder(notifier);
                builder.setInsertPointBefore(op);
                COMPILER_DEBUG(dbg::get() << "Cascade run " << transform->name() << " on " << op->dump() << "{\n");
                transform->run(op, builder);
                COMPILER_DEBUG(dbg::get() << "}\n\n");
            }
        }
    } while (mutated && ++iter < iterLimit);
}

bool CascadeTransform::recurse() const {
    return false;
}

CascadeTransform &CascadeTransform::add(const BaseTransform::Ptr &transform) {
    transforms.emplace_back(transform);
    return *this;
}

CascadeTransform::Ptr CascadeTransform::make(std::string_view commonName, size_t iterLimit) {
    return Ptr(new CascadeTransform(commonName, iterLimit));
}
