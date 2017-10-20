#pragma once

#include <memory>
#include <utility>

namespace variant {
template <typename... Ts>
struct type_list {};

template <typename Variant, typename Variant::tag... ts>
struct tag_list {};

template <typename Variant>
struct make_tag_list_helper {
private:
  using tag_t = typename Variant::tag;
  using underlying_t = std::underlying_type_t<tag_t>;
  using sequence =
      std::make_integer_sequence<underlying_t, underlying_t(tag_t::TAG_END)>;

  template <underlying_t... seq>
  static auto func(std::integer_sequence<underlying_t, seq...>)
      -> tag_list<Variant, tag_t(seq)...> {}

public:
  using type = decltype(func(sequence()));
};

template <typename Variant>
using make_tag_list = typename make_tag_list_helper<Variant>::type;

template <typename Variant, typename Variant::tag Tag>
inline constexpr static char const* tag_name() {
  return Variant::tag_name(
      std::integral_constant<typename Variant::tag, Tag>());
}
template <typename Variant, typename Variant::tag Tag>
using tag_type = decltype(
    Variant::tag_type(std::integral_constant<typename Variant::tag, Tag>()));
template <typename Variant, typename Type>
using type_tag = decltype(Variant::type_tag(std::declval<Type>()));

template <typename Variant>
struct thin {
  using tag_t = typename Variant::tag;

  constexpr tag_t tag() const { return tag_; }
  template <typename Type, typename... Ts>
  static std::unique_ptr<thin> make(Ts&&... ts);

  virtual ~thin() = default;

protected:
  virtual void thin_must_be_virtual_() = 0;
  tag_t tag_;
  thin(tag_t tag_) : tag_(tag_) {}
};

template <typename Variant, typename Type>
struct thin_helper : thin<Variant> {
  using tag = type_tag<Variant, Type>;
  Type value;
  template <typename... Ts>
  thin_helper(Ts&&... ts)
      : thin<Variant>(tag()), value(std::forward<Ts>(ts)...) {}

protected:
  virtual void thin_must_be_virtual_() override {}
};

template <typename Variant>
template <typename Type, typename... Ts>
inline auto thin<Variant>::make(Ts&&... ts) -> std::unique_ptr<thin<Variant>> {
  return std::make_unique<thin_helper<Variant, Type>>(std::forward<Ts>(ts)...);
}

// TODO(ubsan): actually add a fat variant
template <typename Variant>
struct fat {};

template <typename Type, typename Variant>
Type const* thin_cast(thin<Variant> const& x) {
  constexpr auto tag = type_tag<Variant, Type>();
  if (x.tag() == tag) {
    auto const& tmp = static_cast<thin_helper<Variant, Type> const&>(x);
    return &tmp.value;
  } else {
    return nullptr;
  }
}

} // namespace variant

#define declare_variant_member(name)                                           \
  struct name;                                                                 \
  \
static std::integral_constant<tag, tag::name>                                  \
      type_tag(name);                                                          \
  \
static name tag_type(std::integral_constant<tag, tag::name>);                  \
  \
constexpr static char const*                                                   \
  tag_name(std::integral_constant<tag, tag::name>) {                           \
    return #name;                                                              \
  \
}

struct Ast_node {
  enum class tag {
    int_literal,
    plus,
    TAG_END,
  };

  using thin = variant::thin<Ast_node>;
  using fat = variant::fat<Ast_node>;

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
  };
};
