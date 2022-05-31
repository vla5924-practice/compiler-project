#pragma once

#include <forward_list>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>

#include <ast/functions_table.hpp>
#include <ast/node.hpp>
#include <ast/variables_table.hpp>

namespace optimizer {

using VariableValue = std::variant<long int, double>;

struct OptimizerContext {
    std::forward_list<ast::VariablesTable *> variables;
    std::forward_list<std::unordered_map<std::string, VariableValue>> values;
    ast::FunctionsTable &functions;
    ast::Node::Ptr root;

    OptimizerContext(ast::SyntaxTree &tree) : variables(), values(), functions(tree.functions), root(tree.root){};

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
