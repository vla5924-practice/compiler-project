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

    mutable std::unordered_set<std::string> currentCalledFunctions = {"main"};
    mutable std::unordered_set<std::string> calledFunctions = {"main"};

    bool inCurrentCalledList(const Operation::Ptr &op) const {
        return currentCalledFunctions.contains(op->attributes[0].as<std::string>());
    }

    bool inCalledList(const Operation::Ptr &op) const {
        return calledFunctions.contains(op->attributes[0].as<std::string>());
    }

    void getInnerFunctionCallNames(const Operation::Ptr &op) const {
        for (auto &child : op->body) {
            if (child->is<FunctionCallOp>()) {
                currentCalledFunctions.emplace(child->attributes[0].as<std::string>());
            }
            getInnerFunctionCallNames(child);
        }
    };

    void run(const Operation::Ptr &op, OptBuilder &builder) const override {
        if (op->is<ModuleOp>()) {
            for (auto &moduleChild : op->body) {
                if (moduleChild->is<FunctionOp>() && inCurrentCalledList(moduleChild)) {
                    getInnerFunctionCallNames(moduleChild);
                    builder.update(op, []() {});
                    calledFunctions.emplace(moduleChild->attributes[0].as<std::string>());
                    currentCalledFunctions.erase(moduleChild->attributes[0].as<std::string>());
                }
            }
        }

        if (op->is<FunctionOp>() && !inCalledList(op)) {
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
