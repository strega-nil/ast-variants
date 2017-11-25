#include <ast.h>
#include <iostream>
#include <ustd/utility.h>

using namespace ustd;

int evaluate(ast_node::thin const& ast) {
  return match(ast)(
      [](ast_node::int_literal const& il) { return il.value; },
      [](ast_node::plus const& pl) {
        return evaluate(*pl.lhs) + evaluate(*pl.rhs);
      });
}
int evaluate(ast_node::fat const& ast) {
  return match(ast)(
      [](ast_node::int_literal const& il) { return il.value; },
      [](ast_node::plus const& pl) {
        return evaluate(*pl.lhs) + evaluate(*pl.rhs);
      });
}

int main() {
  auto ast = the<ast_node::fat>(ast_node::int_literal(12));
  ast = ast_node::plus(ast_node::int_literal(10), ast_node::int_literal(20));
  std::cout << evaluate(ast) << '\n';
}
