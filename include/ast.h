#pragma once

#include "Variant_helpers.h"

#include <memory>
#include <iostream>

variant_base (Ast_node,
  variant_tags(
    Int_literal,
    Plus
  );

  variant_declare_alternative(Int_literal);
  variant_declare_alternative(Plus);
);

// TODO(ubsan): figure out how to make constructor definition easier

variant_alternative (Ast_node, Int_literal,
  int value;
  Int_literal(int x) : Ast_node(variant_tag), value(x) {}
);

variant_alternative (Ast_node, Plus,
  std::unique_ptr<Ast_node> lhs;
  std::unique_ptr<Ast_node> rhs;

  Plus(std::unique_ptr<Ast_node> lhs, std::unique_ptr<Ast_node> rhs):
    Ast_node(variant_tag), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
);
