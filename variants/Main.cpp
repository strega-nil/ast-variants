#include <iostream>
#include "Ast.h"

int main()
{
  auto ast = Ast_node::make<Plus>(
    Ast_node::make<Int_literal>(1),
    Ast_node::make<Int_literal>(3)
  );
  std::cout << evaluate(*ast);
  char c[10];
  std::cin.getline(c, 9);
}