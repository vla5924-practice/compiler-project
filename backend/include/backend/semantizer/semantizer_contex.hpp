#pragma once

#include <list>
#include <string>
#include <unordered_map>
#include <variant>

#include "error_buffer.hpp"
#include <ast/functions_table.hpp>
#include <ast/node.hpp>
#include <ast/variables_table.hpp>

namespace semantizer {

struct SemantizerContext {
    std::list<ast::VariablesTable *> tables;
    ast::FunctionsTable &functions;
    ErrorBuffer errors;

    SemantizerContext(ast::FunctionsTable &functions_) : tables(), functions(functions_){};
};

} // namespace semantizer