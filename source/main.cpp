#include <ast.h>
#include <iostream>
#include <ustd/utility.h>

using namespace ustd;

int main() {
  auto ast = as<ast_node::fat>(ast_node::int_literal(12));
  ast = ast_node::plus(ast_node::int_literal(10), ast_node::int_literal(20));
  std::cout << evaluate(ast) << '\n';
}
