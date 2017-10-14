#include "Ast.h"
#include <cstdlib>

int evaluate(Ast_node const& ast)
{
  return Ustd::match (ast) (
    [](Int_literal const& node) {
      return node.value;
    },
    [](Plus const& node) {
      return evaluate(*node.lhs) + evaluate(*node.rhs);
    }
  );
}