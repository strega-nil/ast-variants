#pragma once

#include "Type_traits.h"

#include <stdexcept>

namespace Ustd {
  struct Unknown_tag : std::out_of_range {
    Unknown_tag(): out_of_range("Tag is out of range of valid variants") {}
  };

  template <typename Derived, typename Base>
  Derived* try_cast(Base& base)
  {
    if (base.tag() == Derived::variant_tag) {
      return static_cast<Derived*>(&base);
    } else {
      return nullptr;
    }
  }

  template <typename Derived, typename Base>
  Derived const* try_cast(Base const& base)
  {
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
    auto try_all_funcs(F&& f)
    {
      using Derived = copy_cv_t<T, decltype(T::get_type(std::integral_constant<Tag_t, tag>()))>;
      if constexpr (is_callable_v<F, void(Derived&)>) {
        return std::forward<F>(f)(static_cast<Derived&>(*pointer_));
      } else {
        static_assert(false, "The matching was inexhaustive");
      }
    }
    template <Tag_t tag, typename F, typename... Fs>
    auto try_all_funcs(F&& f, Fs&&... fs)
    {
      using Derived = copy_cv_t<T, decltype(T::get_type(std::integral_constant<Tag_t, tag>()))>;
      if constexpr (is_callable_v<F, void(Derived&)>) {
        return std::forward<F>(f)(static_cast<Derived&>(*pointer_));
      } else {
        return try_all_funcs<tag>(std::forward<Fs>(fs)...);
      }
    }

    template <Tag_t tag, typename... Fs>
    auto try_all(Fs&&... fs)
    {
      if constexpr (tag < T::TAG_END - 1) {
        if (tag == pointer_->tag()) {
          return try_all_funcs<tag>(std::forward<Fs>(fs)...);
        } else {
          return try_all<Tag_t(tag + 1)>(std::forward<Fs>(fs)...);
        }
      } else {
        if (tag == pointer_->tag()) {
          return try_all_funcs<tag>(std::forward<Fs>(fs)...);
        } else {
          throw Unknown_tag();
        }
      }
    }
  public:
    Matcher(T& matchee) : pointer_(&matchee) {}

    template <typename... Fs>
    auto operator()(Fs&&... fs)
    {
      //using Return_type = common_return_type_t<Fs...>;
      return try_all<Tag_t(0)>(std::forward<Fs>(fs)...);
    }
  };

  template <typename T>
  auto match(T& matchee)
  {
    return Matcher<T>(matchee);
  }
  template <typename T>
  auto match(T const& matchee)
  {
    return Matcher<T const>(matchee);
  }
  template <typename T>
  auto match(T volatile& matchee)
  {
    return Matcher<T volatile>(matchee);
  }
  template <typename T>
  auto match(T const volatile& matchee)
  {
    return Matcher<T const volatile>(matchee);
  }

  template <typename Derived, typename Base>
  Derived* try_cast(Base&&) = delete;
  template <typename Derived, typename Base>
  Derived const* try_cast(Base const&&) = delete;
}


#define DEFINE_TAG_TYPE(...) enum Tag { __VA_ARGS__, TAG_END }; Tag tag() const { return tag_; }
#define DEFINE_TAG(variant) static struct variant get_type(std::integral_constant<Tag, variant>)
#define DEFINE_MAKE(class_name) \
  template <typename Variant, typename... Ts>\
  static std::unique_ptr<Variant> make(Ts&&... ts)\
  {\
    static_assert(std::is_base_of_v<class_name, Variant>, "make takes a derived Ast_node argument");\
    return std::make_unique<Variant>(std::forward<Ts>(ts)...);\
  }