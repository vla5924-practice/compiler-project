#pragma once

#include <list>
#include <string>
#include <unordered_map>
#include <variant>

#include <ast/functions_table.hpp>
#include <ast/node.hpp>
#include <ast/variables_table.hpp>

namespace optimizer {

using VariableValue = std::variant<long int, double>;

struct OptimizerContext {
    std::list<ast::VariablesTable *> variables;
    std::unordered_map<std::string, VariableValue> values; // TODO: std::map -> std::list<std::map>
    ast::FunctionsTable &functions;

    OptimizerContext(ast::FunctionsTable &functions_) : variables(), values(), functions(functions_){};

    ast::Variable &findVariable(const std::string &name) {
        ast::VariablesTable::iterator varIter;
        for (const auto &table : variables) {
            varIter = table->find(name);
            if (varIter != table->cend()) {
                break;
            }
        }
        return varIter->second;
    }

    ast::Variable &findVariable(const ast::Node::Ptr &varNode) {
        return findVariable(varNode->str());
    }
};

} // namespace optimizer
