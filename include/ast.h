#pragma once

#include <ustd/variant.h>

#include <iostream>
#include <memory>

struct Ast_node {
  variant_tags(Int_literal, Plus);

  variant_declare_alternative(Int_literal);
  variant_declare_alternative(Plus);
  variant_base(Ast_node);
};

// TODO(ubsan): figure out how to make constructor definition easier

struct Ast_node::Int_literal : Ast_node {
  int value;
  Int_literal(int x) : Ast_node(variant_tag), value(x) {}

  variant_alternative(Ast_node, Int_literal);
};

struct Ast_node::Plus : Ast_node {
  std::unique_ptr<Ast_node> lhs;
  std::unique_ptr<Ast_node> rhs;

  Plus(std::unique_ptr<Ast_node> lhs, std::unique_ptr<Ast_node> rhs)
      : Ast_node(variant_tag), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

  variant_alternative(Ast_node, Plus);
};
