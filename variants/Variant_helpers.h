#pragma once

#include "Type_traits.h"

#include <stdexcept>

namespace Ustd {
  struct Inexhaustive_matching : std::logic_error {
    Inexhaustive_matching(): logic_error("Matched variant not covered by passed functions") {}
  };
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

    template <Tag_t tag, typename Ret, typename F>
    Ret try_all_funcs(F&& f)
    {
      using Derived = copy_cv_t<T, decltype(T::get_type(std::integral_constant<Tag_t, tag>()))>;
      //if constexpr (is_callable_v<F, void(Derived&)>) {
        return std::forward<F>(f)(static_cast<Derived&>(*pointer_));
      //} else {
        //throw Inexhaustive_matching();
      //}
    }
    template <Tag_t tag, typename Ret, typename F, typename... Fs>
    Ret try_all_funcs(F&& f, Fs&&... fs)
    {
      using Derived = copy_cv_t<T, decltype(T::get_type(std::integral_constant<Tag_t, tag>()))>;
      if constexpr (is_callable_v<F, void(Derived&)>)
      {
        return std::forward<F>(f)(static_cast<Derived&>(*pointer_));
      } else {
        return try_all_funcs<tag, Ret>(std::forward<Fs>(fs)...);
      }
    }

    template <Tag_t tag, typename Ret, typename... Fs>
    Ret try_all(Fs&&... fs)
    {
      if constexpr (tag < T::TAG_END)
      {
        if (tag == pointer_->tag()) {
          return try_all_funcs<tag, Ret>(std::forward<Fs>(fs)...);
        } else {
          return try_all<Tag_t(tag + 1), Ret>(std::forward<Fs>(fs)...);
        }
      } else {
        throw Unknown_tag();
      }
    }
  public:
    Matcher(T& matchee) : pointer_(&matchee) {}

    template <typename... Fs>
    auto operator()(Fs&&... fs)
    {
      using Return_type = common_return_type_t<Fs...>;
      return try_all<T::TAG_BEGIN, Return_type>(std::forward<Fs>(fs)...);
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