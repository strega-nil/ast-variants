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

int main() {
  auto ast = Ast_node::thin::make<Ast_node::plus>(
      Ast_node::int_literal(0), Ast_node::int_literal(1));
  std::cout << evaluate(ast) << '\n';
}
