#include <ast.h>
#include <iostream>
#include <ustd/utility.h>

using namespace ustd;

int evaluate(Ast_node::thin const& ast) {
  if (auto ptr = thin_cast<Ast_node::int_literal>(ast)) {
    return ptr->value;
  } else if (auto ptr = thin_cast<Ast_node::plus>(ast)) {
    return evaluate(*ptr->lhs) + evaluate(*ptr->rhs);
  } else {
    std::abort();
  }
}

int evaluate(Ast_node::fat const& ast) {
  if (auto ptr = fat_cast<Ast_node::int_literal>(ast)) {
    return ptr->value;
  } else if (auto ptr = fat_cast<Ast_node::plus>(ast)) {
    return evaluate(*ptr->lhs) + evaluate(*ptr->rhs);
  } else {
    std::abort();
  }
}

int main() {
  Ast_node::fat ast{
      Ast_node::plus(Ast_node::int_literal(10), Ast_node::int_literal(20))};
  std::cout << evaluate(ast) << '\n';
}
