#pragma once

#include <type_traits>

namespace ustd {
namespace type_traits {
  template <typename F, typename T, typename Void = void>
  struct is_callable : std::false_type {};

  template <typename F, typename... Ts>
  struct is_callable<
      F, void(Ts...),
      std::void_t<decltype(std::declval<F>()(std::declval<Ts>()...))>>
      : std::true_type {};

  template <typename F, typename T>
  constexpr static bool is_callable_v = is_callable<F, T>();

  // TODO(ubsan): come up with a better name
  template <typename T>
  T declvalcat();

  struct Any_type {
    Any_type() = default;
    template <typename T>
    operator T();
  };

  template <typename T, typename... Ts>
  struct common_type {
    using type = decltype(
        true ? declvalcat<T>()
             : declvalcat<typename common_type<Ts...>::type>());
  };

  template <typename T>
  struct common_type<T> {
    using type = T;
  };

  template <typename... Ts>
  using common_type_t = typename common_type<Ts...>::type;

  template <typename... Fs>
  struct common_return_type {
    using type = common_type_t<decltype(std::declval<Fs>()(Any_type()))...>;
  };

  template <typename... Fs>
  using common_return_type_t = typename common_return_type<Fs...>::type;

  template <typename From, typename To>
  struct copy_cv {
    using type = std::remove_cv_t<To>;
  };
  template <typename From, typename To>
  struct copy_cv<From const, To> {
    using type = std::remove_cv_t<To> const;
  };
  template <typename From, typename To>
  struct copy_cv<From volatile, To> {
    using type = std::remove_cv_t<To> volatile;
  };
  template <typename From, typename To>
  struct copy_cv<From const volatile, To> {
    using type = std::remove_cv_t<To> const volatile;
  };

  template <typename From, typename To>
  using copy_cv_t = typename copy_cv<From, To>::type;

  template <typename... Ts>
  struct type_list {};

  template <typename T, template <typename...> class F>
  struct expand_type_list_helper;

  template <typename... Ts, template <typename...> class F>
  struct expand_type_list_helper<type_list<Ts...>, F> {
    using type = F<Ts...>;
  };
  template <typename T, template <typename...> class F>
  using expand_type_list = typename expand_type_list_helper<T, F>::type;
} // namespace type_traits
}