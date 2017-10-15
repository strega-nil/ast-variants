#include <iostream>
#include <ast.h>
#include "Utility.h"

using namespace ustd;

int evaluate(Ast_node const& ast)
{
  return variant::match(ast) (
    [](Ast_node::Int_literal const& node) {
      return node.value;
    },
    [](Ast_node::Plus const& node) {
      return evaluate(*node.lhs) + evaluate(*node.rhs);
    }
  );
}

int main()
{
  auto ast = Ast_node::Plus(
    std::make_unique<Ast_node::Int_literal>(1),
    std::make_unique<Ast_node::Int_literal>(3)
  );
  std::cout << evaluate(ast) << '\n';
  utility::wait_until_enter();
}