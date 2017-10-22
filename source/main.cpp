#include <ast.h>
#include <iostream>
#include <ustd/utility.h>

using namespace ustd;

int evaluate(ast_node::thin const& ast) {
  if (auto ptr = thin_cast<ast_node::int_literal>(ast)) {
    return ptr->value;
  } else if (auto ptr = thin_cast<ast_node::plus>(ast)) {
    return evaluate(*ptr->lhs) + evaluate(*ptr->rhs);
  } else {
    std::abort();
  }
}

int evaluate(ast_node::fat const& ast) {
  if (auto ptr = fat_cast<ast_node::int_literal>(ast)) {
    return ptr->value;
  } else if (auto ptr = fat_cast<ast_node::plus>(ast)) {
    return evaluate(*ptr->lhs) + evaluate(*ptr->rhs);
  } else {
    std::abort();
  }
}

int main() {
  // make certain we're actually calling the move constructor
  // yay C++17 being sane!
  auto ast = std::move(ast_node::fat(
      ast_node::plus(ast_node::int_literal(10), ast_node::int_literal(20))));
  std::cout << evaluate(ast) << '\n';
}
