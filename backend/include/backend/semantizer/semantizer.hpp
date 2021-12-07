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

    static bool ch(ast::NodeType, ast::TypeId);
    static ast::SyntaxTree &process(ast::SyntaxTree &tree);
    static void test(std::list<ast::Node::Ptr> &children, ast::FunctionsTable &functions);
    static std::vector<ast::TypeId> test1(std::list<ast::Node::Ptr> &children);
    static void test2(ast::Node::Ptr &);
    static void test3(ast::Node::Ptr &, ast::TypeId);
    static void type_conv(ast::Node::Ptr &, ast::NodeType);
};

} // namespace semantizer
