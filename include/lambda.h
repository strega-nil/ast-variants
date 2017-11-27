#pragma once

#include <memory>
#include <string>

#include "ustd/variant.h"

namespace lambda {

struct term {
  declare_variant(term, var, abs, app);

  declare_variant_member(var);

  struct var {
    std::string name;
    var(std::string name) : name(std::move(name)) {}
  };
  declare_variant_member(abs);
  struct abs {
    std::string name;
    std::unique_ptr<thin> body;

    abs(std::string name, std::unique_ptr<thin> body)
        : name(std::move(name)), body(std::move(body)) {}
    template <typename T>
    abs(std::string name, T body)
        : name(std::move(name)), body(thin::make_unique<T>(std::move(body))) {}
  };
  declare_variant_member(app);
  struct app {
    std::unique_ptr<thin> callee;
    std::unique_ptr<thin> parm;

    app(std::unique_ptr<thin> callee, std::unique_ptr<thin> parm)
        : callee(std::move(callee)), parm(std::move(parm)) {}
    template <typename T, typename U>
    app(T callee, U parm)
        : callee(thin::make_unique<T>(std::move(callee))),
          parm(thin::make_unique<U>(std::move(parm))) {}
  };
};

class ast {
public:
  ast(ast&&) = default;
  ast& operator=(ast&&) = default;

private:
  ast() = default;
  ast(ast const&) = delete;
  ast& operator=(ast const&) = delete;

  friend class ast_impl;
};

auto finish(term::fat const&) -> ast;

} // namespace lambda
