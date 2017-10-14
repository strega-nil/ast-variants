#pragma once

#include "Variant_helpers.h"

#include <memory>
#include <iostream>


struct Int_literal;
struct Plus;
struct Ast_node {
  DEFINE_TAG_TYPE(
    Int_literal,
    Plus
  );
  DEFINE_TAG(Int_literal);
  DEFINE_TAG(Plus);

  DEFINE_MAKE(Ast_node);

  virtual ~Ast_node() = 0 {}

protected:
  Tag tag_;
  Ast_node(Tag tag) : tag_(tag) {}
};

// TODO(ubsan): figure out how to make constructor definition easier

struct Int_literal : Ast_node {
public:
  constexpr static auto variant_tag = Ast_node::Int_literal;

  int value;

  Int_literal(int x): Ast_node(variant_tag), value(x) {}
};

struct Plus : Ast_node {
public:
  constexpr static auto variant_tag = Ast_node::Plus;

  std::unique_ptr<Ast_node> lhs;
  std::unique_ptr<Ast_node> rhs;

  Plus(std::unique_ptr<Ast_node> lhs, std::unique_ptr<Ast_node> rhs):
    Ast_node(variant_tag), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
};

int evaluate(Ast_node const&);
