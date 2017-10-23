#include <ast.h>
#include <iostream>
#include <ustd/utility.h>

using namespace ustd;

#if 0
int evaluate(ast_node::thin const& ast) {
  return match(ast)(
      [](ast_node::int_literal const& node) { return node.value; },
      [](ast_node::plus const& node) {
        return evaluate(*node.lhs) + evaluate(*node.rhs);
      });
}

int evaluate(ast_node::fat const& ast) {
  return match(ast)(
      [](ast_node::int_literal const& node) { return node.value; },
      [](ast_node::plus const& node) {
        return evaluate(*node.lhs) + evaluate(*node.rhs);
      });
}
#else
int evaluate(ast_node::thin const& ast) {
  if (auto ptr = variant::get<ast_node::int_literal>(ast)) {
    return ptr->value;
  } else if (auto ptr = variant::get<ast_node::plus>(ast)) {
    return evaluate(*ptr->lhs) + evaluate(*ptr->rhs);
  } else {
    std::abort();
  }
}

int evaluate(ast_node::fat const& ast) {
  if (auto ptr = variant::get<ast_node::int_literal>(ast)) {
    return ptr->value;
  } else if (auto ptr = variant::get<ast_node::plus>(ast)) {
    return evaluate(*ptr->lhs) + evaluate(*ptr->rhs);
  } else {
    std::abort();
  }
}
#endif

int main() {
  auto ast = as<ast_node::fat>(ast_node::int_literal(12));
  ast = ast_node::plus(ast_node::int_literal(10), ast_node::int_literal(20));
  std::cout << evaluate(ast) << '\n';
}
