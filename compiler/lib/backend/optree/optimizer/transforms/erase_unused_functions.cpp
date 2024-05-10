#include "optimizer/transform.hpp"

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/operation.hpp"
#include "optimizer/opt_builder.hpp"
#include <deque>
#include <map>
#include <memory>
#include <string>
#include <unordered_set>

using namespace optree;
using namespace optree::optimizer;

namespace {

struct EraseUnusedFunctions : public Transform<ModuleOp> {
    using Transform::Transform;
    using CallEdges = std::multimap<std::string, std::string>;

    void getInnerFunctionCallNames(const Operation::Ptr &op, const std::string &parentName, CallEdges &edges) const {
        for (auto &child : op->body) {
            auto funcOp = child->as<FunctionCallOp>();
            if (funcOp) {
                edges.emplace(parentName, funcOp.name());
            }
            getInnerFunctionCallNames(child, parentName, edges);
        }
    };

    void run(const Operation::Ptr &op, OptBuilder &builder) const override {
        CallEdges edges = {};
        for (auto &moduleChild : op->body) {
            auto funcOp = moduleChild->as<FunctionOp>();
            if (funcOp)
                getInnerFunctionCallNames(moduleChild, funcOp.name(), edges);
        }
        auto range = edges.equal_range("main");
        std::unordered_set<std::string> usedFunctions = {"main"};
        std::deque<std::pair<std::string, std::string>> queue(range.first, range.second);
        while (!queue.empty()) {
            if (!usedFunctions.contains(queue.front().second)) {
                usedFunctions.emplace(queue.front().second);
                auto innerRange = edges.equal_range(queue.front().second);
                queue.insert(queue.end(), innerRange.first, innerRange.second);
            }
            queue.pop_front();
        }

        for (auto it : utils::advanceEarly(op->body.begin(), op->body.end())) {
            if (!usedFunctions.contains(it->as<FunctionOp>().name())) {
                builder.erase(it);
            }
        }
    }
};

} // namespace

namespace optree {
namespace optimizer {

BaseTransform::Ptr createEraseUnusedFunctions() {
    return std::make_shared<EraseUnusedFunctions>();
}

} // namespace optimizer
} // namespace optree
