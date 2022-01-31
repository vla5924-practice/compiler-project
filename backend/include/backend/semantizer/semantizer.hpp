#pragma once

#include <ast/syntax_tree.hpp>
#include <ast/types.hpp>

namespace semantizer {

class Semantizer {
  public:
    Semantizer() = delete;
    Semantizer(const Semantizer &) = delete;
    Semantizer(Semantizer &&) = delete;
    ~Semantizer() = delete;

    static void process(ast::SyntaxTree &tree);

  private:
    static void parseFunctions(std::list<ast::Node::Ptr> &children, ast::FunctionsTable &functions);
    static std::vector<ast::TypeId> getFunctionArguments(std::list<ast::Node::Ptr> &children);
    static void processBranchRoot(ast::Node::Ptr &node, ast::FunctionsTable &functions);
    static void processExpression(ast::Node::Ptr &node, ast::TypeId, ast::Node::Ptr &branch);
    static void pushTypeConversion(ast::Node::Ptr &node, ast::NodeType);
    static void pushTypeConversion(ast::Node::Ptr &node, ast::TypeId);
    static ast::TypeId searchVariable(ast::Node::Ptr &node, const std::string &name);
};

} // namespace semantizer
