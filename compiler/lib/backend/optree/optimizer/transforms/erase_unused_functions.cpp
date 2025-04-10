#include <deque>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_set>

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/utils/helpers.hpp"
#include "compiler/utils/language.hpp"

#include "optimizer/opt_builder.hpp"
#include "optimizer/transform.hpp"

using namespace optree;
using namespace optree::optimizer;

namespace {

struct EraseUnusedFunctions : public Transform<ModuleOp> {
    using Transform::Transform;
    using CallEdges = std::map<std::string, std::unordered_set<std::string>>;

    std::string_view name() const override {
        return "EraseUnusedFunctions";
    }

    void getInnerFunctionCallNames(const Operation::Ptr &op, const std::string &parentName, CallEdges &edges) const {
        for (auto &child : op->body) {
            auto funcOp = child->as<FunctionCallOp>();
            if (funcOp) {
                edges[parentName].emplace(funcOp.name());
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
        auto &mainFunctions = edges[utils::language::funcMain];
        std::unordered_set<std::string> usedFunctions = {utils::language::funcMain};
        std::deque<std::string> queue(mainFunctions.begin(), mainFunctions.end());
        while (!queue.empty()) {
            auto &name = queue.front();
            if (!usedFunctions.contains(name)) {
                usedFunctions.emplace(name);
                auto &innerFunctions = edges[name];
                queue.insert(queue.end(), innerFunctions.begin(), innerFunctions.end());
            }
            queue.pop_front();
        }

        for (const auto &op : utils::advanceEarly(op->body)) {
            if (!usedFunctions.contains(op->as<FunctionOp>().name())) {
                builder.erase(op);
            }
        }
    }

    bool recurse() const override {
        return false;
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
