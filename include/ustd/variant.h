#pragma once

#include "meta.h"

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

    template <typename Tag, Tag Current, template <Tag> class F, typename... Ts>
    static auto tag_visit_helper(Tag tag, Ts&&... ts) {
      if constexpr (tag_valid(tag_increment(Current))) {
        if (tag == Current) {
          return F<Current>::func(std::forward<Ts>(ts)...);
        } else {
          return tag_visit_helper<Tag, tag_increment(Current), F>(
              tag, std::forward<Ts>(ts)...);
        }
      } else {
        if (tag == Current) {
          return F<Current>::func(std::forward<Ts>(ts)...);
        } else {
          // tag is not a valid Tag
          std::abort();
        }
      }
    }

    template <typename Tag, template <Tag> class F, typename... Ts>
    static auto tag_visit(Tag tag, Ts&&... ts) {
      return tag_visit_helper<Tag, Tag(0), F>(tag, std::forward<Ts>(ts)...);
    }

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
  template <typename Variant, typename Variant::tag X>
  struct tag_list_types_helper<tag_list<Variant, X>> {
    using type = meta::type_list<tag_type<Variant, X>>;
  };
  template <
      typename Variant, typename Variant::tag X, typename Variant::tag... Xs>
  struct tag_list_types_helper<tag_list<Variant, X, Xs...>> {
    using type = meta::union_type_lists<
        meta::type_list<tag_type<Variant, X>>,
        typename tag_list_types_helper<tag_list<Variant, Xs...>>::type>;
  };
  template <typename Tag_list>
  using tag_list_types = typename tag_list_types_helper<Tag_list>::type;

  template <typename Variant>
  struct thin {
    using variant_t = Variant;
    using tag_t = typename variant_t::tag;

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

  namespace impl {
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

  template <typename Variant>
  struct fat {
    using variant_t = Variant;
    using tag_t = typename variant_t::tag;
    constexpr static auto size = meta::expand_type_list<
        tag_list_types<make_tag_list<variant_t>>, impl::max_size>::value;
    constexpr static auto align = meta::expand_type_list<
        tag_list_types<make_tag_list<variant_t>>, impl::max_align>::value;

    template <typename T>
    fat(T t) : tag_(type_tag<variant_t, T>()) {
      new (&storage_) T(std::move(t));
    }

    fat() = delete;

    tag_t tag() const { return tag_; }
    void* raw_storage() { return static_cast<void*>(&storage_); }
    void const* raw_storage() const {
      return static_cast<void const*>(&storage_);
    }
    void volatile* raw_storage() volatile {
      return static_cast<void volatile*>(&storage_);
    }
    void const volatile* raw_storage() const volatile {
      return static_cast<void const volatile*>(&storage_);
    }

  private:
    tag_t tag_;
    std::aligned_storage_t<size, align> storage_;

    template <tag_t tag>
    struct smf_helper {
      static void func(fat& self, fat const& other) {
        using type = tag_type<variant_t, tag>;
        new (&self.storage_)
            type(reinterpret_cast<type const&>(other.storage_));
      }
      static void func(fat& self, fat&& other) {
        using type = tag_type<variant_t, tag>;
        new (&self.storage_) type(reinterpret_cast<type&&>(other.storage_));
      }
    };
    template <tag_t tag>
    struct destructor_helper {
      static void func(fat& self) {
        using type = tag_type<variant_t, tag>;
        reinterpret_cast<type&>(self.storage_).~type();
      }
    };

  public:
    fat(fat const& other) : tag_(other.tag_) {
      impl::tag_visit<tag_t, smf_helper>(tag_, *this, other);
    }
    fat(fat&& other) : tag_(other.tag_) {
      impl::tag_visit<tag_t, smf_helper>(tag_, *this, std::move(other));
    }
    fat& operator=(fat const& other) & {
      this->~fat();
      try {
        new (this) fat(other);
      } catch (...) {
        std::abort();
      }
      return *this;
    }
    fat& operator=(fat&& other) & {
      this->~fat();
      try {
        new (this) fat(std::move(other));
      } catch (...) {
        std::abort();
      }
      return *this;
    }
    ~fat() { impl::tag_visit<tag_t, destructor_helper>(tag_, *this); }
  };

  template <typename Variant>
  class matcher_thin {
    using tag_t = typename Variant::tag;
    using variant_t = std::remove_cv_t<Variant>;
    using thin_t = thin<variant_t>;
    using pointed_t = meta::copy_cv_t<Variant, thin_t>;
    pointed_t* pointer_;

    template <tag_t Tag>
    using get_type = meta::copy_cv_t<Variant, tag_type<variant_t, Tag>>;
    template <tag_t Tag>
    using get_helper = meta::copy_cv_t<
        Variant, typename thin_t::template helper<tag_type<variant_t, Tag>>>;

    template <tag_t Tag>
    struct call_correct_function {
      template <typename F, typename... Fs>
      static auto func(pointed_t& matchee, F&& f, Fs&&... fs) {
        if constexpr (meta::is_callable_v<F, void(get_type<Tag>&)>) {
          return std::forward<F>(f)(
              static_cast<get_helper<Tag>&>(matchee).value);
        } else {
          return call_correct_function::func(matchee, std::forward<Fs>(fs)...);
        }
      }
    };

  public:
    matcher_thin(pointed_t& matchee) : pointer_(&matchee) {}

    template <typename... Fs>
    auto operator()(Fs&&... fs) {
      return impl::tag_visit<tag_t, call_correct_function>(
          pointer_->tag(), *pointer_, std::forward<Fs>(fs)...);
    }
  };

  template <typename Variant>
  class matcher_fat {
    using tag_t = typename Variant::tag;
    using variant_t = std::remove_cv_t<Variant>;
    using fat_t = fat<variant_t>;
    using pointed_t = meta::copy_cv_t<Variant, fat_t>;
    pointed_t* pointer_;

    template <tag_t Tag>
    using get_type = meta::copy_cv_t<Variant, tag_type<variant_t, Tag>>;

    template <tag_t Tag>
    struct call_correct_function {
      template <typename F, typename... Fs>
      static auto func(pointed_t& matchee, F&& f, Fs&&... fs) {
        if constexpr (meta::is_callable_v<F, void(get_type<Tag>&)>) {
          return std::forward<F>(f)(
              *reinterpret_cast<get_type<Tag>*>(matchee.raw_storage()));
        } else {
          return call_correct_function::func(matchee, std::forward<Fs>(fs)...);
        }
      }
    };

  public:
    matcher_fat(pointed_t& matchee) : pointer_(&matchee) {}

    template <typename... Fs>
    auto operator()(Fs&&... fs) {
      return impl::tag_visit<tag_t, call_correct_function>(
          pointer_->tag(), *pointer_, std::forward<Fs>(fs)...);
    }
  };

  template <typename T>
  auto match(variant::thin<T>& matchee) {
    return variant::matcher_thin<T>(matchee);
  }
  template <typename T>
  auto match(variant::thin<T> const& matchee) {
    return variant::matcher_thin<T const>(matchee);
  }
  template <typename T>
  auto match(variant::thin<T> volatile& matchee) {
    return variant::matcher_thin<T volatile>(matchee);
  }
  template <typename T>
  auto match(variant::thin<T> const volatile& matchee) {
    return variant::matcher_thin<T const volatile>(matchee);
  }

  template <typename T>
  auto match(variant::fat<T>& matchee) {
    return variant::matcher_fat<T>(matchee);
  }
  template <typename T>
  auto match(variant::fat<T> const& matchee) {
    return variant::matcher_fat<T const>(matchee);
  }
  template <typename T>
  auto match(variant::fat<T> volatile& matchee) {
    return variant::matcher_fat<T volatile>(matchee);
  }
  template <typename T>
  auto match(variant::fat<T> const volatile& matchee) {
    return variant::matcher_fat<T const volatile>(matchee);
  }

  namespace impl {
    template <typename Variant, typename Type>
    using thin_helper = typename variant::thin<Variant>::template helper<Type>;
  }

  template <typename Type, typename Variant>
  constexpr auto unsafe_get(thin<Variant>& x) {
    return &static_cast<impl::thin_helper<Variant, Type>&>(x).value;
  }
  template <typename Type, typename Variant>
  constexpr auto unsafe_get(thin<Variant> const& x) {
    return &static_cast<impl::thin_helper<Variant, Type> const&>(x).value;
  }
  template <typename Type, typename Variant>
  constexpr auto unsafe_get(thin<Variant> volatile& x) {
    return &static_cast<impl::thin_helper<Variant, Type> volatile&>(x).value;
  }
  template <typename Type, typename Variant>
  constexpr auto unsafe_get(thin<Variant> const volatile& x) {
    auto&& tmp =
        static_cast<impl::thin_helper<Variant, Type> const volatile&>(x);
    return &tmp.value;
  }
  template <typename Type, typename Variant>
  auto unsafe_get(fat<Variant>& x) {
    return reinterpret_cast<Type*>(x.raw_storage());
  }
  template <typename Type, typename Variant>
  auto unsafe_get(fat<Variant> const& x) {
    return reinterpret_cast<Type const*>(x.raw_storage());
  }
  template <typename Type, typename Variant>
  auto unsafe_get(fat<Variant> volatile& x) {
    return reinterpret_cast<Type volatile*>(x.raw_storage());
  }
  template <typename Type, typename Variant>
  auto unsafe_get(fat<Variant> const volatile& x) {
    return reinterpret_cast<Type const volatile*>(x.raw_storage());
  }

  template <typename Type, typename Variant>
  constexpr void unsafe_get(thin<Variant> const volatile&& x) = delete;
  template <typename Type, typename Variant>
  constexpr void unsafe_get(fat<Variant> const volatile&& x) = delete;

  template <typename Type, typename T>
  constexpr auto get(T&& x) -> decltype(unsafe_get<Type>(x)) {
    using tag_t = type_tag<typename std::decay_t<T>::variant_t, Type>;
    if (x.tag() == tag_t()) {
      return unsafe_get<Type>(std::forward<T>(x));
    } else {
      return nullptr;
    }
  }

} // namespace variant
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

#ifdef USTD_MSVC_COMPATIBLE
#define _variant_internal_extern_templates_nonmovable(xtrn, name)              \
  xtrn template constexpr name::tag ustd::variant::thin<name>::tag() const;    \
  xtrn template ustd::variant::thin<name>::thin(tag_t tag_);                   \
                                                                               \
  xtrn template name::tag ustd::variant::fat<name>::tag() const;               \
  xtrn template void* ustd::variant::fat<name>::raw_storage();                 \
  xtrn template void const* ustd::variant::fat<name>::raw_storage() const;     \
  xtrn template ustd::variant::fat<name>::~fat();                              \
                                                                               \
  xtrn template class ustd::variant::matcher_thin<name>;                       \
  xtrn template class ustd::variant::matcher_thin<name const>;                 \
  xtrn template class ustd::variant::matcher_fat<name>;                        \
  xtrn template class ustd::variant::matcher_fat<name const>
#elif USTD_STD_COMPATIBLE
#define _variant_internal_extern_templates_nonmovable(xtrn, name)              \
  xtrn template name::tag ustd::variant::thin<name>::tag() const;              \
  xtrn template ustd::variant::thin<name>::thin(tag_t tag_);                   \
                                                                               \
  xtrn template name::tag ustd::variant::fat<name>::tag() const;               \
  xtrn template void* ustd::variant::fat<name>::raw_storage();                 \
  xtrn template void const* ustd::variant::fat<name>::raw_storage() const;     \
  xtrn template ustd::variant::fat<name>::~fat();                              \
                                                                               \
  xtrn template class ustd::variant::matcher_thin<name>;                       \
  xtrn template class ustd::variant::matcher_thin<name const>;                 \
  xtrn template class ustd::variant::matcher_fat<name>;                        \
  xtrn template class ustd::variant::matcher_fat<name const>
#endif

#define _variant_internal_extern_templates_noncopyable(xtrn, name)             \
  _variant_internal_extern_templates_nonmovable(xtrn, name);                   \
  xtrn template ustd::variant::fat<name>::fat(fat&& other);                    \
  xtrn template ustd::variant::fat<name>& ::ustd::variant::fat<name>::         \
  operator=(fat&& other)&

#define _variant_internal_extern_templates(xtrn, name)                         \
  _variant_internal_extern_templates_noncopyable(xtrn, name);                  \
  xtrn template ustd::variant::fat<name>::fat(fat const& other);               \
  xtrn template ustd::variant::fat<name>& ::ustd::variant::fat<name>::         \
  operator=(fat const& other)&

#if defined(USTD_MSVC_COMPATIBLE) || defined(USTD_STD_COMPATIBLE)
#define variant_declare_extern_templates_nonmovable(name)                      \
  _variant_internal_extern_templates_nonmovable(extern, name)
#define variant_declare_extern_templates_noncopyable(name)                     \
  _variant_internal_extern_templates_noncopyable(extern, name)
#define variant_declare_extern_templates(name)                                 \
  _variant_internal_extern_templates(extern, name)

#define variant_define_extern_templates_nonmovable(name)                       \
  _variant_internal_extern_templates_nonmovable(, name)
#define variant_define_extern_templates_noncopyable(name)                      \
  _variant_internal_extern_templates_noncopyable(, name)
#define variant_define_extern_templates(name)                                  \
  _variant_internal_extern_templates(, name)
#else
#define variant_declare_extern_templates_nonmovable(name)
#define variant_declare_extern_templates_noncopyable(name)
#define variant_declare_extern_templates(name)

#define variant_define_extern_templates_nonmovable(name)
#define variant_define_extern_templates_noncopyable(name)
#define variant_define_extern_templates(name)
#endif