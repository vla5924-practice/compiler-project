#include <memory>
#include <string_view>

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/helpers.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/optree/types.hpp"
#include "compiler/utils/helpers.hpp"

#include "optimizer/opt_builder.hpp"
#include "optimizer/transform.hpp"

#include <algorithm>
#include <iostream>
#include <map>
#include <set>

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

    void sinkOperation(const Operation::Ptr &child, RegionMap &regionMap, OptBuilder &builder) const {
        for (const auto &result : child->results) {
            std::vector<Region> usingIn{};
            bool notFound = false;
            for (const auto &use : result->uses) {
                auto user = use.lock();
                if (auto search = regionMap.find(user); search != regionMap.end()) {
                    usingIn.push_back(search->second);
                } else {
                    notFound = true;
                }
            }
            if (!notFound) {
                auto position =
                    std::min_element(usingIn.begin(), usingIn.end(),
                                     [](const Region &lhs, const Region &rhs) { return lhs.id < rhs.id; });
                if (position == usingIn.end()) {
                    continue;
                }
                Region minRegion = *position;
                uint32_t minRegionId = minRegion.id;
                if (minRegionId <= regionMap[child].id) {
                    continue;
                }
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
        std::set<Region, decltype([](const Region &lhs, const Region &rhs) { return lhs.id < rhs.id; })> test;
        for (const auto &[rOp, region] : regionMap) {
            test.insert(region);
            std::cout << rOp->name << " : " << region.id << std::endl;
        }
        std::cout << " : " << std::endl;

        for (const auto &elem : test) {
            std::cout << elem.owner->name << " : " << elem.id << std::endl;
        }
        dfs(op, regionMap, builder);
        std::cout << " ______________________ " << std::endl;

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
