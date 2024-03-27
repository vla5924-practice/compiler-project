#pragma once

#include <forward_list>
#include <map>
#include <string>

#include "compiler/optree/builder.hpp"
#include "compiler/optree/operation.hpp"

namespace optree {

namespace converter {

struct ConverterContext {
    Operation::Ptr op;
    std::forward_list<std::map<std::string, Value::Ptr>> variables;
    Builder builder;

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

    void saveVariable(const std::string &name, Value::Ptr value) {
        variables.front()[name] = value;
    }

    Value::Ptr findVariable(const std::string &name) {
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

} // namespace optree
