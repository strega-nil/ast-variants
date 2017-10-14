#ifndef VARIANT_AST_H
#define VARIANT_AST_H

#include "Variant_helpers.h"

#include <memory>
#include <iostream>

struct Int_literal;
struct Plus;
struct Ast_node {
  enum Tag {
    Int_literal,
    Plus,
    TAG_END,
    TAG_BEGIN = Int_literal,
  };
  auto tag() const { return tag_; }

  static struct Int_literal get_type(std::integral_constant<Tag, Int_literal>);
  static struct Plus get_type(std::integral_constant<Tag, Plus>);

  template <typename Variant, typename... Ts>
  static std::unique_ptr<Variant> make(Ts&&... ts)
  {
    static_assert(std::is_base_of_v<Ast_node, Variant>, "make takes a derived Ast_node argument");
    return std::make_unique<Variant>(std::forward<Ts>(ts)...);
  }

  virtual ~Ast_node() = 0 {}

protected:
  Tag tag_;
  Ast_node(Tag tag) : tag_(tag) {}
};

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



#endif
