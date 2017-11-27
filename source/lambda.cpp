#include <lambda.h>

namespace lambda {

class ast_impl {
public:
  static auto finish(term::fat const& tm) -> ast { return ast(); }
};

auto finish(term::fat const& tm) -> ast { return ast_impl::finish(tm); }

} // namespace lambda
