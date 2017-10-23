#include <ast.h>

variant_define_extern_templates_noncopyable(ast_node);

int evaluate(ast_node::thin const& ast) {
  return match(ast)(
      [](ast_node::int_literal const& node) { return node.value; },
      [](ast_node::plus const& node) {
        return evaluate(*node.lhs) + evaluate(*node.rhs);
      });
}

int evaluate(ast_node::fat const& ast) {
  return match(ast)(
      [](ast_node::int_literal const& node) { return node.value; },
      [](ast_node::plus const& node) {
        return evaluate(*node.lhs) + evaluate(*node.rhs);
      });
}