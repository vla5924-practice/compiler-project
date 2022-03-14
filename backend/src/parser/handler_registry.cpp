#include "parser/handler_registry.hpp"

#include "parser/handlers/branch_root_handler.hpp"
#include "parser/handlers/elif_statement_handler.hpp"
#include "parser/handlers/expression_handler.hpp"
#include "parser/handlers/function_arguments_handler.hpp"
#include "parser/handlers/function_definition_handler.hpp"
#include "parser/handlers/if_statement_handler.hpp"
#include "parser/handlers/program_root_handler.hpp"
#include "parser/handlers/return_statement_handler.hpp"
#include "parser/handlers/variable_declaration_handler.hpp"
#include "parser/handlers/while_statement_handler.hpp"

using namespace parser;

HandlerRegistry::HandlerRegistry() {
    registerHandler(ast::NodeType::BranchRoot, new BranchRootHandler);
    registerHandler(ast::NodeType::ElifStatement, new ElifStatementHandler);
    registerHandler(ast::NodeType::Expression, new ExpressionHandler);
    registerHandler(ast::NodeType::FunctionArguments, new FunctionArgumentsHandler);
    registerHandler(ast::NodeType::FunctionDefinition, new FunctionDefinitionHandler);
    registerHandler(ast::NodeType::IfStatement, new IfStatementHandler);
    registerHandler(ast::NodeType::ProgramRoot, new ProgramRootHandler);
    registerHandler(ast::NodeType::ReturnStatement, new ReturnStatementHandler);
    registerHandler(ast::NodeType::VariableDeclaration, new VariableDeclarationHandler);
    registerHandler(ast::NodeType::WhileStatement, new WhileStatementHandler);
}

void HandlerRegistry::registerHandler(const ast::NodeType &nodeType, BaseHandler *handler) {
    (*this)[nodeType].reset(handler);
}
