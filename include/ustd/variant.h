#pragma once

#include "type_traits.h"

#include <stdexcept>

namespace ustd::variant {
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
} // namespace impl

struct Unknown_tag : std::out_of_range {
  Unknown_tag() : out_of_range("Tag is out of range of valid variants") {}
};

template <typename Derived, typename Base>
Derived* try_cast(Base& base) {
  if (base.tag() == Derived::variant_tag) {
    return static_cast<Derived*>(&base);
  } else {
    return nullptr;
  }
}

template <typename Derived, typename Base>
Derived const* try_cast(Base const& base) {
  if (base.tag() == Derived::variant_tag) {
    return static_cast<Derived const*>(&base);
  } else {
    return nullptr;
  }
}

template <typename T>
class Matcher {
  using Tag_t = typename T::Tag;
  T* pointer_;

  template <Tag_t tag, typename F>
  auto try_all_funcs(F&& f) {
    using Derived = type_traits::copy_cv_t<
        T, decltype(T::get_type(std::integral_constant<Tag_t, tag>()))>;
    if constexpr (type_traits::is_callable_v<F, void(Derived&)>) {
      return std::forward<F>(f)(static_cast<Derived&>(*pointer_));
    } else {
      static_assert(false, "The matching was inexhaustive");
    }
  }
  template <Tag_t tag, typename F, typename... Fs>
  auto try_all_funcs(F&& f, Fs&&... fs) {
    using Derived = type_traits::copy_cv_t<
        T, decltype(T::get_type(std::integral_constant<Tag_t, tag>()))>;
    if constexpr (type_traits::is_callable_v<F, void(Derived&)>) {
      return std::forward<F>(f)(static_cast<Derived&>(*pointer_));
    } else {
      return try_all_funcs<tag>(std::forward<Fs>(fs)...);
    }
  }

  template <Tag_t tag, typename... Fs>
  auto try_all(Fs&&... fs) {
    if constexpr (impl::tag_valid(impl::tag_increment(tag))) {
      if (tag == pointer_->tag()) {
        return try_all_funcs<tag>(std::forward<Fs>(fs)...);
      } else {
        return try_all<impl::tag_increment(tag)>(std::forward<Fs>(fs)...);
      }
    } else {
      if (tag == pointer_->tag()) {
        return try_all_funcs<tag>(std::forward<Fs>(fs)...);
      } else {
        std::abort();
      }
    }
  }

public:
  Matcher(T& matchee) : pointer_(&matchee) {}

  template <typename... Fs>
  auto operator()(Fs&&... fs) {
    return try_all<Tag_t(0)>(std::forward<Fs>(fs)...);
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

template <typename Derived, typename Base>
Derived* try_cast(Base&&) = delete;
template <typename Derived, typename Base>
Derived const* try_cast(Base const&&) = delete;
} // namespace ustd::variant

#define variant_tags(...)                                                      \
  enum class Tag { __VA_ARGS__, TAG_END };                                     \
  Tag tag() const { return tag_; }

#define variant_declare_alternative(variant)                                   \
  struct variant;                                                              \
  static struct variant get_type(std::integral_constant<Tag, Tag::variant>)

#define variant_base(name)                                                     \
  \
public:                                                                        \
  virtual ~name() {}                                                           \
  \
protected:                                                                     \
  Tag tag_;                                                                    \
  virtual void _variant_force_pure_virtual() = 0;                              \
  \
name(Tag tag)                                                                  \
      : tag_(tag) {}

#define variant_alternative(base, name)                                        \
  constexpr static auto variant_tag = base::Tag::name;                         \
  void _variant_force_pure_virtual() override {}
