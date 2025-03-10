#pragma once

#include <cstddef>
#include <forward_list>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <variant>

#include "compiler/ast/node.hpp"
#include "compiler/optree/builder.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/optree/types.hpp"
#include "compiler/optree/value.hpp"
#include "compiler/utils/error_buffer.hpp"

#include "compiler/frontend/converter/converter_error.hpp"

namespace converter {

struct ConverterContext {
    struct LocalVariable {
        using NumElementsStorage = std::variant<std::monostate, optree::Value::Ptr, size_t>;

        optree::Value::Ptr value = {};
        bool needsLoad = true;
        NumElementsStorage numElements = {};
    };

    optree::Operation::Ptr op;
    std::unordered_map<std::string, optree::Type::Ptr> functions;
    std::forward_list<std::unordered_map<std::string, LocalVariable>> variables;
    optree::Builder builder;
    ErrorBuffer errors;

    template <typename AdaptorType, typename... Args>
    AdaptorType insert(Args... args) {
        return builder.insert<AdaptorType>(std::forward<Args>(args)...);
    }

    void goInto(const optree::Operation::Ptr &rootOp) {
        op = rootOp;
        builder.setInsertPointAtBodyEnd(op);
    }

    void goParent() {
        if (op->parent == nullptr)
            return;
        goInto(op->parent);
    }

    void enterScope() {
        variables.emplace_front();
    }

    void exitScope() {
        variables.pop_front();
    }

    void saveVariable(const std::string &name, optree::Value::Ptr value, bool needsLoad = true,
                      const LocalVariable::NumElementsStorage &numElements = {}) {
        variables.front().emplace(std::piecewise_construct, std::forward_as_tuple(name),
                                  std::forward_as_tuple(value, needsLoad, numElements));
    }

    const LocalVariable *findVariable(const std::string &name) {
        for (auto &scope : variables)
            if (scope.contains(name))
                return &scope[name];
        return nullptr;
    }

    bool wouldBeRedeclaration(const std::string &name) {
        return variables.front().contains(name);
    }

    void pushError(const ast::Node::Ptr &node, const std::string &message) {
        errors.push<ConverterError>(node, message);
    }
};

} // namespace converter
