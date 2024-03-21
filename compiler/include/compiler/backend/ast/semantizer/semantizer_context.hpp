#pragma once

#include <list>

#include "compiler/ast/functions_table.hpp"
#include "compiler/ast/node.hpp"
#include "compiler/ast/variables_table.hpp"
#include "compiler/utils/error_buffer.hpp"

#include "compiler/backend/ast/semantizer/semantizer_error.hpp"

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
