#include "optimizer/transform.hpp"

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/operation.hpp"
#include "optimizer/opt_builder.hpp"
#include <memory>
#include <string>
#include <unordered_set>

using namespace optree;
using namespace optree::optimizer;

namespace {

struct EraseUnusedFunctions : public Transform<ModuleOp, FunctionOp> {
    using Transform::Transform;

    inline static std::unordered_set<std::string> currentCalledFunctions = {"main"};
    inline static std::unordered_set<std::string> calledFunctions = {"main"};

    static bool inCurrentCalledList(const Operation::Ptr &op) {
        return currentCalledFunctions.contains(op->attr(0).as<std::string>());
    }

    static bool inCalledList(const Operation::Ptr &op) {
        return calledFunctions.contains(op->attr(0).as<std::string>());
    }

    static void getInnerFunctionCallNames(const Operation::Ptr &op) {
        for (auto &child : op->body) {
            if (child->is<FunctionCallOp>()) {
                currentCalledFunctions.emplace(child->attr(0).as<std::string>());
            }
            getInnerFunctionCallNames(child);
        }
    };

    static void addFunction(const std::string& name) {
        calledFunctions.emplace(name);
        currentCalledFunctions.erase(name);
    };

    void run(const Operation::Ptr &op, OptBuilder &builder) const override {
        if (op->is<ModuleOp>()) {
            for (auto &moduleChild : op->body) {
                if (moduleChild->is<FunctionOp>() && inCurrentCalledList(moduleChild)) {
                    getInnerFunctionCallNames(moduleChild);
                    builder.update(op);
                    addFunction(moduleChild->attr(0).as<std::string>());
                }
            }
        }

        if (op->is<FunctionOp>() && !inCalledList(op)) {
            builder.erase(op);
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
