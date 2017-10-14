#include "Ast.h"
#include <cstdlib>

int evaluate(Ast_node const& ast)
{
  #if 0
  if (auto node = try_cast<Int_literal>(ast)) {
    return node->value;
  } else if (auto node = try_cast<Plus>(ast)) {
    return evaluate(*node->lhs) + evaluate(*node->rhs);
  } else {
    std::abort();
  }
  #else
  return Ustd::match (ast) (
    [](Int_literal const& node) {
      return node.value;
    },
    [](Plus const& node) {
      return evaluate(*node.lhs) + evaluate(*node.rhs);
    }
  );
  #endif
}