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
    static void processBranchRoot(ast::Node::Ptr &, ast::FunctionsTable &functions);
    static void processExpression(ast::Node::Ptr &, ast::TypeId, ast::Node::Ptr &);
    static void pushTypeConversion(ast::Node::Ptr &, ast::NodeType);
    static void pushTypeConversion(ast::Node::Ptr &, ast::TypeId);
};

} // namespace semantizer
