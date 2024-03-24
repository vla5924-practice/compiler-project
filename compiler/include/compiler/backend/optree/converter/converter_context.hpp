#pragma once

#include <forward_list>
#include <map>
#include <string>

#include "compiler/optree/operation.hpp"

namespace optree {

namespace converter {

struct ConverterContext {
    Operation::Ptr op;
    std::forward_list<std::map<std::string, Value::Ptr>> variables;

    template <typename AdaptorType, typename... Args>
    std::pair<Operation::Ptr, AdaptorType> addToBody(Args... args) {
        auto newOp = Operation::make<AdaptorType>(op);
        op->addToBody(newOp);
        AdaptorType adaptor(newOp);
        adaptor.init(std::forward<Args>(args)...);
        return std::make_pair(newOp, adaptor);
    }

    void goParent() {
        op = op->parent;
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
};

} // namespace converter

} // namespace optree
