#pragma once

#include "type_traits.h"

#include <memory>
#include <utility>

namespace ustd {
namespace variant {
  namespace impl {

    template <typename T>
    constexpr T tag_increment(T const& tag) {
      return static_cast<T>(static_cast<std::underlying_type_t<T>>(tag) + 1);
    }
    template <typename T>
    constexpr bool tag_valid(T const& tag) {
      using underlying = std::underlying_type_t<T>;
      constexpr auto tag_end = static_cast<underlying>(T::TAG_END);
      return static_cast<underlying>(tag) < tag_end;
    }

    template <typename T>
    constexpr auto max(T t) {
      return t;
    }
    template <typename T, typename... Ts>
    constexpr auto max(T t, Ts... ts) {
      auto rest = max(ts...);
      return t < rest ? rest : t;
    }

    template <typename... Ts>
    struct max_size {
      static constexpr auto value = max(sizeof(Ts)...);
    };
    template <typename... Ts>
    struct max_align {
      static constexpr auto value = max(alignof(Ts)...);
    };
  } // namespace impl

  template <typename Variant, typename Variant::tag... Ts>
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

  template <typename Tag_list>
  struct tag_list_types_helper;
  template <typename Variant, typename Variant::tag... Ts>
  struct tag_list_types_helper<tag_list<Variant, Ts...>> {
    using type = type_traits::type_list<tag_type<Variant, Ts>...>;
  };
  template <typename Tag_list>
  using tag_list_types = typename tag_list_types_helper<Tag_list>::type;

  template <typename Variant>
  struct thin {
    using tag_t = typename Variant::tag;

    template <typename Type>
    struct helper;

    constexpr tag_t tag() const { return tag_; }
    template <typename Type, typename... Ts>
    static std::unique_ptr<helper<Type>> make_unique(Ts&&... ts);
    template <typename Type, typename... Ts>
    constexpr static helper<Type> make(Ts&&... ts);

    virtual ~thin() = default;

  protected:
    virtual void thin_must_be_virtual_() = 0;
    tag_t tag_;
    thin(tag_t tag_) : tag_(tag_) {}
  };

  template <typename Variant>
  template <typename Type>
  struct thin<Variant>::helper : thin<Variant> {
    using tag = type_tag<Variant, Type>;
    Type value;

    template <typename... Ts>
    helper(Ts&&... ts) : thin<Variant>(tag()), value(std::forward<Ts>(ts)...) {}

  protected:
    virtual void thin_must_be_virtual_() override {}
  };

  template <typename Variant>
  template <typename Type, typename... Ts>
  inline auto thin<Variant>::make_unique(Ts&&... ts)
      -> std::unique_ptr<helper<Type>> {
    return std::make_unique<helper<Type>>(std::forward<Ts>(ts)...);
  }
  template <typename Variant>
  template <typename Type, typename... Ts>
  constexpr inline auto thin<Variant>::make(Ts&&... ts) -> helper<Type> {
    return helper<Type>(std::forward<Ts>(ts)...);
  }

  // TODO(ubsan): actually add a fat variant
  // NOTE(ubsan): should probably inherit from thin<Variant>
  template <typename Variant>
  struct fat {
    using tag_t = typename Variant::tag;

  private:
    tag_t tag_;
    std::aligned_storage<
        type_traits::expand_type_list<
            tag_list_types<make_tag_list<Variant>>, impl::max_size>::value,
        type_traits::expand_type_list<
            tag_list_types<make_tag_list<Variant>>, impl::max_align>::value>
        storage_;
  };

} // namespace variant

template <typename Type, typename Variant>
constexpr Type const* thin_cast(variant::thin<Variant> const& x) {
  using helper = typename variant::thin<Variant>::template helper<Type>;
  constexpr auto tag = variant::type_tag<Variant, Type>();
  if (x.tag() == tag) {
    auto const& tmp = static_cast<helper const&>(x);
    return &tmp.value;
  } else {
    return nullptr;
  }
}

template <typename Type, typename Variant>
constexpr Type* thin_cast(variant::thin<Variant>& x) {
  using helper = typename variant::thin<Variant>::template helper<Type>;
  constexpr auto tag = variant::type_tag<Variant, Type>();
  if (x.tag() == tag) {
    auto& tmp = static_cast<helper&>(x);
    return &tmp.value;
  } else {
    return nullptr;
  }
}

} // namespace ustd

#define declare_variant(name, ...)                                             \
  enum class tag {                                                             \
    __VA_ARGS__,                                                               \
    TAG_END,                                                                   \
  };                                                                           \
  \
using thin = ustd::variant::thin<name>;                                        \
  \
using fat = ustd::variant::fat<name>

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

#if 0
  template <typename T>
  class Matcher {
    using Tag_t = typename T::Tag;
    T* pointer_;

    template <Tag_t tag>
    using get_type = type_traits::copy_cv_t<
        T, decltype(T::get_type(std::integral_constant<Tag_t, tag>()))>;
    template <Tag_t tag, typename F>
    using callable = std::integral_constant<
        bool, type_traits::is_callable_v<F, void(get_type<tag>)>>;

    template <Tag_t tag, typename F, typename... Fs>
    auto try_all_funcs(std::true_type, F&& f, Fs&&...) {
      return std::forward<F>(f)(static_cast<get_type<tag>&>(*pointer_));
    }
    template <Tag_t tag, typename F, typename F2, typename... Fs>
    auto try_all_funcs(std::false_type, F&&, F2&& f2, Fs&&... fs) {
      return try_all_funcs<tag>(
          callable<tag, F2>(), std::forward<F2>(f2), std::forward<Fs>(fs)...);
    }
    template <Tag_t tag, typename F>
    auto try_all_funcs(std::false_type, F&&) {
      static_assert(sizeof(F) < 0, "The matching was inexhaustive");
    }

    template <Tag_t tag>
    using next_tag_valid =
        std::integral_constant<bool, impl::tag_valid(impl::tag_increment(tag))>;

    template <Tag_t tag, typename F, typename... Fs>
    auto try_all(std::true_type, F&& f, Fs&&... fs) {
      if (tag == pointer_->tag()) {
        return try_all_funcs<tag>(
            callable<tag, F>(), std::forward<F>(f), std::forward<Fs>(fs)...);
      } else {
        constexpr auto new_tag = impl::tag_increment(tag);
        return try_all<new_tag>(
            next_tag_valid<new_tag>(), std::forward<Fs>(fs)...);
      }
    }

    template <Tag_t tag, typename F, typename... Fs>
    auto try_all(std::false_type, F&& f, Fs&&... fs) {
      if (tag == pointer_->tag()) {
        return try_all_funcs<tag>(
            callable<tag, F>(), std::forward<F>(f), std::forward<Fs>(fs)...);
      } else {
        std::abort();
      }
    }

  public:
    Matcher(T& matchee) : pointer_(&matchee) {}

    template <typename... Fs>
    auto operator()(Fs&&... fs) {
      constexpr auto tag = Tag_t(0);
      return try_all<tag>(next_tag_valid<tag>(), std::forward<Fs>(fs)...);
    }
  };

  template <typename T>
  auto match(T& matchee) {
    return Matcher<T>(matchee);
  }
  template <typename T>
  auto match(T const& matchee) {
    return Matcher<T const>(matchee);
  }
  template <typename T>
  auto match(T volatile& matchee) {
    return Matcher<T volatile>(matchee);
  }
  template <typename T>
  auto match(T const volatile& matchee) {
    return Matcher<T const volatile>(matchee);
  }
#endif