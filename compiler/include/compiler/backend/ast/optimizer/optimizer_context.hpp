#pragma once

#include <cstdint>
#include <forward_list>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>

#include "compiler/ast/functions_table.hpp"
#include "compiler/ast/node.hpp"
#include "compiler/ast/variables_table.hpp"

#include "compiler/backend/ast/optimizer/optimizer_options.hpp"

namespace optimizer {

using VariableValue = std::variant<int64_t, double>;

struct OptimizerContext {
    std::forward_list<ast::VariablesTable *> variables;
    std::forward_list<std::unordered_map<std::string, VariableValue>> values;
    ast::FunctionsTable &functions;
    ast::Node::Ptr root;
    OptimizerOptions options;

    OptimizerContext(ast::FunctionsTable &functions, const OptimizerOptions &options)
        : variables(), values(), functions(functions), options(options){};

    ast::Variable &findVariable(const std::string &name) {
        for (auto &table : variables) {
            auto varIter = table->find(name);
            if (varIter != table->cend()) {
                return varIter->second;
            }
        }

        throw std::runtime_error("Variable not found");
    }

    ast::Variable &findVariable(const ast::Node::Ptr &varNode) {
        return findVariable(varNode->str());
    }

    VariableValue &findVariableValue(const std::string &name) {
        for (auto &table : values) {
            auto varIter = table.find(name);
            if (varIter != table.cend()) {
                return varIter->second;
            }
        }

        throw std::runtime_error("Variable not found");
    }

    bool hasVariable(const std::string &name) {
        for (auto &table : values)
            if (table.find(name) != table.cend())
                return true;

        return false;
    }
};

} // namespace optimizer
