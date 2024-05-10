#include "optimizer/transform.hpp"

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/operation.hpp"
#include "optimizer/opt_builder.hpp"
#include <iostream>
#include <memory>
#include <unordered_set>

using namespace optree;
using namespace optree::optimizer;

namespace {

struct EraseUnusedFun : public Transform<ModuleOp, FunctionOp> {
    using Transform::Transform;

    mutable std::unordered_set<std::string> calledFunctions = {"main"};
    mutable std::list<Operation::Ptr> erasedFunctions = {};

    bool isFunctionName(const Operation::Ptr &op, const std::string &functionName) const {
        return op->attributes[0].as<std::string>() == functionName;
    }

    bool inCalledList(const Operation::Ptr &op) const {
        return calledFunctions.contains(op->attributes[0].as<std::string>());
    }

    bool inErasedList(const Operation::Ptr &op) const {
        return std::find(erasedFunctions.cbegin(), erasedFunctions.cend(), op) != erasedFunctions.cend();
    }

    void run(const Operation::Ptr &op, OptBuilder &builder) const override {
        std::cout << op->name << " "
                  << "\n";
        if (op->is<ModuleOp>()) {
            for (auto &moduleChild : op->body) {
                if (moduleChild->is<FunctionOp>() && inCalledList(moduleChild)) {
                    calledFunctions.emplace("main");
                    for (auto &child : moduleChild->body) {
                        if (child->is<FunctionCallOp>()) {
                            calledFunctions.emplace(child->attributes[0].as<std::string>());
                        }
                    }
                }
            }
        }

        if (op->is<FunctionOp>() && !inCalledList(op)) {
            erasedFunctions.emplace_back(op);
            builder.erase(op);
            op->parent->body.erase(op->position);
        }
    }
};

} // namespace

namespace optree {
namespace optimizer {

BaseTransform::Ptr createEraseUnusedFunctions() {
    return std::make_shared<EraseUnusedFun>();
}

} // namespace optimizer
} // namespace optree
