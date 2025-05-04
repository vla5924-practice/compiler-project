#include <algorithm>
#include <cstdint>
#include <map>
#include <memory>
#include <string_view>
#include <vector>

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/utils/helpers.hpp"

#include "optimizer/opt_builder.hpp"
#include "optimizer/transform.hpp"

using namespace optree;
using namespace optree::optimizer;

namespace {

struct ControlFlowSinkHelper {
    struct Region {
        uint32_t id;
        Operation::Ptr owner;
    };

    using RegionMap = std::map<Operation::Ptr, Region>;

    ControlFlowSinkHelper(OptBuilder &builder) : builder(builder){};

    void fillRegionMap(const Operation::Ptr &op, uint32_t &scope) {
        if (!op->body.empty())
            scope++;
        for (auto &child : op->body) {
            regionMap[child] = {scope, op};
            fillRegionMap(child, scope);
        }
    };

    bool isParentChild(const Operation::Ptr &child, const Operation::Ptr &parentCandidate) {
        if (child == parentCandidate)
            return true;
        auto parent = child->parent;
        while (parent) {
            if (parent == parentCandidate)
                return true;
            parent = parent->parent;
        }
        return false;
    }

    void sinkOperation(const Operation::Ptr &child) {
        uint32_t childPos = regionMap[child].id;
        if (child->results.size() != 1)
            return;
        for (const auto &result : child->results) {
            std::vector<Region> usingIn;
            bool found = true;
            for (const auto &use : result->uses) {
                auto user = use.lock();
                if (auto it = regionMap.find(user); it != regionMap.end()) {
                    if (childPos < it->second.id)
                        usingIn.emplace_back(it->second);
                    else
                        found = false;
                } else {
                    found = false;
                }
            }

            if (found && !usingIn.empty()) {
                auto minRegion = usingIn[0];
                const auto &candidate = usingIn[0].owner;
                bool siblings = false;
                for (const auto &pos : usingIn) {
                    if (pos.id < minRegion.id)
                        minRegion = pos;
                    const auto &posOwner = pos.owner;
                    if (!isParentChild(posOwner, candidate) || !isParentChild(candidate, posOwner)) {
                        siblings = true;
                    }
                }
                if (siblings) {
                    continue;
                }
                uint32_t minRegionId = minRegion.id;
                if (auto thenOp = minRegion.owner->as<ThenOp>()) {
                    if (auto it = std::find_if(usingIn.begin(), usingIn.end(),
                                               [&minRegion](const Region &reg) { return reg.id == minRegion.id + 1; });
                        it != usingIn.end()) {
                        continue;
                    }
                }

                builder.setInsertPointBefore(*minRegion.owner->body.begin());
                auto newOp = builder.clone(child);
                regionMap.erase(child);
                regionMap[newOp] = minRegion;
                builder.replace(child, newOp);
            }
        }
    }

    void traverseOps(const Operation::Ptr &op) {
        for (const auto &child : utils::advanceEarly(op->body)) {
            traverseOps(child);
            sinkOperation(child);
        }
    }

    RegionMap regionMap;
    OptBuilder &builder;
};

struct SinkControlFlowOps : public Transform<FunctionOp> {
    using Transform::Transform;

    std::string_view name() const override {
        return "SinkControlFlowOps";
    }

    void run(const Operation::Ptr &op, OptBuilder &builder) const override {
        auto funcOp = op->as<FunctionOp>();
        auto context = ControlFlowSinkHelper(builder);

        uint32_t scope = 0;
        for (const auto &child : op->body) {
            auto ifOp = child->as<IfOp>();
            if (ifOp) {
                context.fillRegionMap(child, scope);
            } else {
                context.regionMap[child] = {0, op};
            }
        }
        context.traverseOps(op);
    }
};

} // namespace

namespace optree {
namespace optimizer {

BaseTransform::Ptr createSinkControlFlowOps() {
    return std::make_shared<SinkControlFlowOps>();
}

} // namespace optimizer
} // namespace optree
