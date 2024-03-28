#pragma once

#include <forward_list>
#include <map>
#include <string>

#include "compiler/optree/builder.hpp"
#include "compiler/optree/operation.hpp"

namespace converter {

struct ConverterContext {
    optree::Operation::Ptr op;
    std::forward_list<std::map<std::string, optree::Value::Ptr>> variables;
    optree::Builder builder;

    template <typename AdaptorType, typename... Args>
    AdaptorType insert(Args... args) {
        return builder.insert<AdaptorType>(std::forward<Args>(args)...);
    }

    void goParent() {
        op = op->parent;
        builder.setInsertPointAtBodyEnd(op);
    }

    void enterScope() {
        variables.emplace_front();
    }

    void exitScope() {
        variables.pop_front();
    }

    void saveVariable(const std::string &name, optree::Value::Ptr value) {
        variables.front()[name] = value;
    }

    optree::Value::Ptr findVariable(const std::string &name) {
        for (auto &scope : variables)
            if (scope.contains(name))
                return scope[name];
        return {};
    }

    bool wouldBeRedeclaration(const std::string &name) {
        return variables.front().contains(name);
    }
};

} // namespace converter
