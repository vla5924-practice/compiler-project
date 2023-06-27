#pragma once

#include <list>

#include "error_buffer.hpp"
#include "semantizer_error.hpp"

#include <ast/functions_table.hpp>
#include <ast/node.hpp>
#include <ast/variables_table.hpp>

namespace semantizer {

struct SemantizerContext {
    std::list<ast::VariablesTable *> variables;
    ast::FunctionsTable &functions;
    ErrorBuffer errors;
    ast::TypeId currentFunctionType;

    SemantizerContext(ast::FunctionsTable &functions_) : variables(), functions(functions_){};

    ast::TypeId findVariable(const ast::Node::Ptr &node) {
        ast::TypeId type = ast::BuiltInTypes::BuiltInTypesCount;
        const std::string &variableName = node->str();
        for (const auto &table : variables) {
            auto tableName = table->find(variableName);
            if (tableName != table->cend()) {
                type = tableName->second.type;
                break;
            }
        }

        if (type == ast::BuiltInTypes::BuiltInTypesCount) {
            errors.push<SemantizerError>(*node, variableName + " was not declared in this scope");
        }

        return type;
    };
};

} // namespace semantizer
