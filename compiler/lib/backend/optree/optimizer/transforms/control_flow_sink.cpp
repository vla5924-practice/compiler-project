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

struct ControlFlowSinkOps : public Transform<FunctionOp> {
    using Transform::Transform;

    std::string_view name() const override {
        return "ControlFlowSinkOps";
    }

    struct Region {
        uint32_t id;
        Operation::Ptr owner;
    };
    using RegionMap = std::map<Operation::Ptr, Region>;

    void fillRegionMap(const Operation::Ptr &op, RegionMap &regionMap, uint32_t &scope) const {
        if (!op->body.empty())
            scope++;
        for (auto &child : op->body) {
            regionMap[child] = {scope, op};
            fillRegionMap(child, regionMap, scope);
        }
    };

    bool isParentChild(const Operation::Ptr &child, const Operation::Ptr &parentCandidate) const {
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

    void sinkOperation(const Operation::Ptr &child, RegionMap &regionMap, OptBuilder &builder) const {
        uint32_t childPos = regionMap[child].id;
        for (const auto &result : child->results) {
            std::vector<Region> usingIn{}; // can use set in this?
            bool notFound = false;
            for (const auto &use : result->uses) {
                auto user = use.lock();
                if (auto search = regionMap.find(user); search != regionMap.end()) {
                    if (childPos < search->second.id)
                        usingIn.emplace_back(search->second);
                    else
                        notFound = true;
                } else {
                    notFound = true;
                }
            }

            if (!notFound && !usingIn.empty()) {
                auto minRegion = usingIn[0];
                const auto &candidate = usingIn[0].owner;
                // cheking that operations are family
                bool isBrothers = false;
                for (const auto &pos : usingIn) {
                    if (pos.id < minRegion.id)
                        minRegion = pos;
                    const auto &posOwner = pos.owner;
                    if (!(isParentChild(posOwner, candidate) || isParentChild(candidate, posOwner))) {
                        isBrothers = true;
                    }
                }
                if (isBrothers) {
                    continue;
                }
                uint32_t minRegionId = minRegion.id;
                if (auto thenOp = minRegion.owner->as<ThenOp>()) {
                    if (auto search =
                            std::find_if(usingIn.begin(), usingIn.end(),
                                         [&minRegion](const Region &reg) { return reg.id == minRegion.id + 1; });
                        search != usingIn.end()) {
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

    void dfs(const Operation::Ptr &op, RegionMap &regionMap, OptBuilder &builder) const {
        for (const auto &child : utils::advanceEarly(op->body)) {
            dfs(child, regionMap, builder);
            sinkOperation(child, regionMap, builder);
        }
    }

    void run(const Operation::Ptr &op, OptBuilder &builder) const override {
        auto funcOp = op->as<FunctionOp>();
        uint32_t rootId = 0;
        RegionMap regionMap;
        uint32_t scope = 0;
        for (const auto &child : op->body) {
            auto ifOp = child->as<IfOp>();
            if (ifOp) {
                fillRegionMap(child, regionMap, scope);
            } else {
                regionMap[child] = {rootId, op};
            }
        }
        dfs(op, regionMap, builder);
    }
};

} // namespace

namespace optree {
namespace optimizer {

BaseTransform::Ptr createControlFlowSinkOps() {
    return std::make_shared<ControlFlowSinkOps>();
}

} // namespace optimizer
} // namespace optree
