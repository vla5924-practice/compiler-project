#pragma once

#include <list>

#include "error_buffer.hpp"
#include "semantizer_error.hpp"
#include <ast/functions_table.hpp>
#include <ast/node.hpp>
#include <ast/variables_table.hpp>

namespace semantizer {

struct SemantizerContext {
    std::list<ast::VariablesTable *> tables;
    ast::FunctionsTable &functions;
    ErrorBuffer errors;

    SemantizerContext(ast::FunctionsTable &functions_) : tables(), functions(functions_){};

    ast::TypeId searchVariable(const ast::Node::Ptr &node) {
        ast::TypeId type = ast::BuiltInTypes::BuiltInTypesCount;
        for (const auto &table : tables) {
            ast::VariablesTable::const_iterator table_name = table->find(node->str());
            if (table_name != table->cend()) {
                type = table_name->second;
                break;
            }
        }

        if (type == ast::BuiltInTypes::BuiltInTypesCount) {
            errors.push<SemantizerError>(*node, node->str() + " was not declared in this scope");
        }

        return type;
    };
};

} // namespace semantizer
