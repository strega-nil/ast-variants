#pragma once

#include <memory>

#include "ustd/variant.h"

struct ast_node {
  declare_variant(ast_node, int_literal, plus);

  declare_variant_member(int_literal);
  struct int_literal {
    int value;
    int_literal(int value) : value(value) {}
  };
  declare_variant_member(plus);
  struct plus {
    std::unique_ptr<thin> lhs;
    std::unique_ptr<thin> rhs;
    plus(std::unique_ptr<thin> lhs, std::unique_ptr<thin> rhs)
        : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    template <typename T, typename U>
    plus(T lhs, U rhs)
        : lhs(thin::make_unique<T>(std::move(lhs))),
          rhs(thin::make_unique<U>(std::move(rhs))) {}
  };
};
