#pragma once
#include "../lpl/lpl.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <iterator>
#include <numeric>
#include <optional>
#include <ratio>

#define decltype_t(t) typename std::decay_t<decltype(t)>::type

#define type_from(x) ::ltl::type_v<decltype(x)>
#define decay_from(x) ::ltl::type_v<std::decay_t<decltype(x)>>

#define typed_static_assert(f) static_assert(decltype(f){})
#define typed_static_assert_msg(f, msg) static_assert(decltype(f){}, msg)
#define if_constexpr(c) if constexpr (decltype(c){})
#define else_if_constexpr(c) else if constexpr (decltype(c){})

#define compile_time_error(msg, T) static_assert(::ltl::always_false<T>, msg);

#define FWD(x) ::std::forward<decltype(x)>(x)

namespace ltl {
template <typename> constexpr bool always_false = false;

/////////////////////// decay_reference_wrapper
template <typename T> struct decay_reference_wrapper {
  using type = std::decay_t<T>;
};

template <typename T>
struct decay_reference_wrapper<std::reference_wrapper<T>> {
  using type = T &;
};

template <typename T>
using decay_reference_wrapper_t = typename decay_reference_wrapper<T>::type;

///////////////////// overloader
template <typename... Fs> struct overloader : Fs... {
  constexpr overloader(Fs &&... fs) : Fs{std::move(fs)}... {}

  using Fs::operator()...;
};

////////////////////// crtp
/*
template <typename T, template <typename...> typename crtpType> struct crtp {
  T &underlying() { return static_cast<T &>(*this); }

  constexpr const T &underlying() const {
    return static_cast<const T &>(*this);
  }
};*/
#define ENABLE_CRTP(DerivedType)                                               \
  DerivedType &underlying() { return static_cast<DerivedType &>(*this); }      \
  constexpr const DerivedType &underlying() const {                            \
    return static_cast<const DerivedType &>(*this);                            \
  }

///////////////////// bool
template <bool v> struct bool_t {
  static constexpr bool value = v;
  constexpr operator bool() const { return v; }
};

using false_t = bool_t<false>;
using true_t = bool_t<true>;
} // namespace ltl

constexpr ltl::false_t false_v;
constexpr ltl::true_t true_v;

namespace ltl {
template <bool v> constexpr bool_t<v> bool_v{};

#define OP(op)                                                                 \
  template <bool v1, bool v2>                                                  \
  [[nodiscard]] constexpr bool_t<v1 op v2> operator op(bool_t<v1>,             \
                                                       bool_t<v2>) {           \
    return {};                                                                 \
  }

LPL_MAP(OP, ==, !=, &&, ||)

#undef OP

template <bool v>[[nodiscard]] constexpr bool_t<!v> operator!(bool_t<v>) {
  return {};
}

///////////////////////// type
template <typename T> struct type_t {
  using type = T;
  constexpr type_t() = default;
};

template <typename T> constexpr type_t<T> type_v{};

template <typename T1, typename T2>
[[nodiscard]] constexpr false_t operator==(type_t<T1>, type_t<T2>) {
  return {};
}

template <typename T>
[[nodiscard]] constexpr true_t operator==(type_t<T>, type_t<T>) {
  return {};
}

template <typename T1, typename T2>
[[nodiscard]] constexpr true_t operator!=(type_t<T1>, type_t<T2>) {
  return {};
}

template <typename T>
[[nodiscard]] constexpr false_t operator!=(type_t<T>, type_t<T>) {
  return {};
}

////////////////////// number
template <int N> struct number_t { constexpr static int value = N; };

namespace detail {
template <char... _digits>[[nodiscard]] constexpr int digits_to_int() {
  constexpr char digits[] = {_digits...};
  int result = 0;
  for (int digit : digits) {
    result *= 10;
    result += digit - '0';
  }
  return result;
}
} // namespace detail

#define OP(op)                                                                 \
  template <int N1, int N2>                                                    \
  [[nodiscard]] constexpr number_t<(N1 op N2)> operator op(number_t<N1>,       \
                                                           number_t<N2>) {     \
    return {};                                                                 \
  }
LPL_MAP(OP, +, -, *, /, %, &, |, ^, <<, >>)
#undef OP

#define OP(op)                                                                 \
  template <int N1, int N2>                                                    \
  [[nodiscard]] constexpr bool_t<(N1 op N2)> operator op(number_t<N1>,         \
                                                         number_t<N2>) {       \
    return {};                                                                 \
  }

LPL_MAP(OP, ==, !=, <, <=, >, >=)
#undef OP

#define OP(op)                                                                 \
  template <int N>                                                             \
  [[nodiscard]] constexpr number_t<(op N)> operator op(number_t<N>) {          \
    return {};                                                                 \
  }
LPL_MAP(OP, ~, +, -)
#undef OP
} // namespace ltl

template <int N> constexpr ltl::number_t<N> number_v{};
template <char... digits>[[nodiscard]] constexpr auto operator""_n() {
  return number_v<ltl::detail::digits_to_int<digits...>()>;
}

namespace ltl {
[[nodiscard]] constexpr auto bool_to_number(false_t) { return 0_n; }
[[nodiscard]] constexpr auto bool_to_number(true_t) { return 1_n; }

template <typename T>[[nodiscard]] constexpr auto max(T a) { return a; }

template <typename T1, typename T2, typename... Ts>
[[nodiscard]] constexpr auto max(T1 a, T2 b, Ts... ts) {
  if_constexpr(a > b) return ::ltl::max(a, ts...);
  else return ::ltl::max(b, ts...);
}

template <typename T>[[nodiscard]] constexpr auto min(T a) { return a; }
template <typename T1, typename T2, typename... Ts>
[[nodiscard]] constexpr auto min(T1 a, T2 b, Ts... ts) {
  if_constexpr(a < b) return ::ltl::min(a, ts...);
  else return ::ltl::min(b, ts...);
}

/////////////////////////// Traits
#define TRAIT(name)                                                            \
  template <typename... Ts>                                                    \
      [[nodiscard]] constexpr bool_t<std::LPL_CAT(name, _v) < Ts...>>          \
      LPL_CAT(name, _Impl)(type_t<Ts>...) {                                    \
    return {};                                                                 \
  }                                                                            \
  template <typename... Ts>                                                    \
  [[nodiscard]] constexpr auto LPL_CAT(name, _Impl)(Ts && ... ts) {            \
    return LPL_CAT(name, _Impl)(decay_from(ts)...);                            \
  }                                                                            \
  constexpr auto name = [](auto &&... xs) constexpr {                          \
    return LPL_CAT(name, _Impl)(FWD(xs)...);                                   \
  };

// Primary type categories
LPL_MAP(TRAIT, is_void, is_null_pointer, is_integral, is_floating_point,
        is_enum, is_union, is_class, is_function, is_pointer,
        is_member_object_pointer, is_member_function_pointer)

// Composite type categories
LPL_MAP(TRAIT, is_fundamental, is_arithmetic, is_scalar, is_object, is_compound,
        is_member_pointer)

// Type properties
LPL_MAP(TRAIT, is_trivial, is_trivially_copyable, is_standard_layout,
        /*has_unique_object_representations,*/ is_empty, is_polymorphic,
        is_abstract, is_final, is_aggregate, is_signed, is_unsigned)

// Supported operations
LPL_MAP(TRAIT, is_default_constructible, is_trivially_default_constructible,
        is_nothrow_default_constructible, is_copy_constructible,
        is_trivially_copy_constructible, is_nothrow_copy_constructible,
        is_move_constructible, is_trivially_move_constructible,
        is_nothrow_move_constructible)

LPL_MAP(TRAIT, is_copy_assignable, is_trivially_copy_assignable,
        is_nothrow_copy_assignable, is_move_assignable,
        is_trivially_move_assignable, is_nothrow_move_assignable)

LPL_MAP(TRAIT, is_assignable, is_trivially_assignable, is_nothrow_assignable,
        is_swappable_with, is_nothrow_swappable_with)

LPL_MAP(TRAIT, is_destructible, is_trivially_destructible,
        is_nothrow_destructible, has_virtual_destructor, is_swappable,
        is_nothrow_swappable)

LPL_MAP(TRAIT, is_constructible, is_trivially_constructible,
        is_nothrow_constructible)

// Type relationships
LPL_MAP(TRAIT, is_same, is_base_of, is_convertible)

LPL_MAP(TRAIT, is_invocable, is_invocable_r, is_nothrow_invocable,
        is_nothrow_invocable_r)

#undef TRAIT

#define TRAIT_REFERENCE(name)                                                  \
  template <typename T>                                                        \
      [[nodiscard]] constexpr bool_t<std::LPL_CAT(name, _v) < T>>              \
      LPL_CAT(name, _Impl)(type_t<T>) {                                        \
    return {};                                                                 \
  }                                                                            \
  template <typename T>                                                        \
  [[nodiscard]] constexpr auto LPL_CAT(name, _Impl)(T && x) {                  \
    return LPL_CAT(name, _Impl)(type_from(x));                                 \
  }                                                                            \
  constexpr auto name = [](auto &&x) constexpr {                               \
    return LPL_CAT(name, _Impl)(FWD(x));                                       \
  };

#define TRAIT_CVNESS(name)                                                     \
  template <typename T>                                                        \
      [[nodiscard]] constexpr bool_t<std::LPL_CAT(name, _v) < T>>              \
      LPL_CAT(name, _Impl)(type_t<T>) {                                        \
    return {};                                                                 \
  }                                                                            \
  template <typename T>                                                        \
  [[nodiscard]] constexpr auto LPL_CAT(name, _Impl)(T &&) {                    \
    return LPL_CAT(name, _Impl)(type_v<std::remove_reference_t<T>>);           \
  }                                                                            \
  constexpr auto name = [](auto &&x) constexpr {                               \
    return LPL_CAT(name, _Impl)(FWD(x));                                       \
  };

// Reference / cv-ness
LPL_MAP(TRAIT_REFERENCE, is_lvalue_reference, is_rvalue_reference)
LPL_MAP(TRAIT_CVNESS, is_const, is_volatile, is_array)

#undef TRAIT_REFERENCE
#undef TRAIT_CVNESS

#define TRAIT(name)                                                            \
  template <typename T>                                                        \
      [[nodiscard]] constexpr number_t<std::LPL_CAT(name, _v) < T>>            \
      name(type_t<T>) {                                                        \
    return {};                                                                 \
  }

// Property queries
template <typename T, int N = 0>
[[nodiscard]] constexpr number_t<std::extent_v<T, N>>
extent(type_t<T>, number_t<N> = number_t<N>{}) {
  return {};
}

LPL_MAP(TRAIT, alignment_of, rank)

#undef TRAIT

#define TRAIT(name)                                                            \
  template <typename T>                                                        \
      [[nodiscard]] constexpr type_t<std::LPL_CAT(name, _t) < T>>              \
      LPL_CAT(name, _Impl)(type_t<T>) {                                        \
    return {};                                                                 \
  }                                                                            \
  template <typename T>                                                        \
  [[nodiscard]] constexpr auto LPL_CAT(name, _Impl)(T &&) {                    \
    return LPL_CAT(name, _Impl)(type_v<T>);                                    \
  }                                                                            \
  constexpr auto name = [](auto &&x) constexpr {                               \
    return LPL_CAT(name, _Impl)(FWD(x));                                       \
  };

// const-volatibility specifiers
LPL_MAP(TRAIT, remove_cv, remove_const, remove_volatile, add_cv, add_const,
        add_volatile)

// references
LPL_MAP(TRAIT, remove_reference, add_lvalue_reference, add_rvalue_reference)

// pointers
LPL_MAP(TRAIT, remove_pointer, add_pointer)

// Sign modifiers
LPL_MAP(TRAIT, make_signed, make_unsigned)

// Arrays
LPL_MAP(TRAIT, remove_extent, remove_all_extents)

TRAIT(decay)
#undef TRAIT

///////////////////////// is_valid

template <typename F> constexpr auto is_valid(F f) {
  return overloader{[f](auto &&... xs) -> decltype(f(FWD(xs)...), true_v) {
                      (void)f;
                      return {};
                    },
                    [](...) -> false_t { return {}; }};
}

#define LTL_WRITE_AUTO_WITH_COMMA_IMPL(x) , auto &&x

#define LTL_WRITE_AUTO_IMPL(x, ...)                                            \
  (auto &&x LPL_MAP(LTL_WRITE_AUTO_WITH_COMMA_IMPL, __VA_ARGS__))

#define LTL_ENSURE_NOT_EMPTY(...) (__VA_ARGS__, ())
#define IS_VALID(variables, ...)                                               \
  ltl::is_valid(                                                               \
      [] LPL_IDENTITY(LTL_WRITE_AUTO_IMPL LTL_ENSURE_NOT_EMPTY variables)      \
          -> decltype(__VA_ARGS__, void()) {})

template <typename T>[[nodiscard]] constexpr auto is_iterableImpl(type_t<T>) {
  constexpr auto trait = IS_VALID((x), std::begin(FWD(x)), std::end(FWD(x)));
  return decltype(trait(std::declval<T>())){};
}

template <typename T>[[nodiscard]] constexpr auto is_iterableImpl(T &&x) {
  return is_iterableImpl(type_from(x));
}

constexpr auto is_iterable = [](auto &&x) constexpr {
  return is_iterableImpl(FWD(x));
};

#define LTL_MAKE_IS_KIND(type, name, conceptName, templateType)                \
  template <typename T> struct LPL_CAT(name, ImplStruct) : false_t {};         \
  template <templateType... Ts>                                                \
  struct LPL_CAT(name, ImplStruct)<type<Ts...>> : true_t {};                   \
  template <typename T>                                                        \
      constexpr bool_t<LPL_CAT(name, ImplStruct) < T>::value >                 \
      LPL_CAT(name, Impl)(type_t<T>) {                                         \
    return {};                                                                 \
  }                                                                            \
  template <typename T>                                                        \
      constexpr bool_t<LPL_CAT(name, ImplStruct) < std::decay_t<T>>::value >   \
      LPL_CAT(name, Impl)(T &&) {                                              \
    return {};                                                                 \
  }                                                                            \
  constexpr auto name = [](auto &&x) constexpr noexcept {                      \
    return LPL_CAT(name, Impl)(FWD(x));                                        \
  };                                                                           \
  template <typename T>                                                        \
  constexpr auto conceptName =                                                 \
      bool_v<LPL_CAT(name, ImplStruct) < std::decay_t<T>>::value >

LTL_MAKE_IS_KIND(type_t, is_type_t, IsType, typename);
LTL_MAKE_IS_KIND(number_t, is_number_t, IsNumber, int);
LTL_MAKE_IS_KIND(bool_t, is_bool_t, IsBool, bool);

template <typename T> constexpr auto is_type(type_t<T> type) {
  return ltl::overloader{
      [type](auto &&x) constexpr
          ->std::enable_if_t<IsType<decltype(x)>, decltype(type == FWD(x))>{
              (void)type;
  return {};
}
, [type](auto &&x) constexpr
  -> std::enable_if_t<!IsType<std::decay_t<decltype(x)>>,
                      decltype(type == decay_from(x))> {
  (void)type;
  return {};
}
}; // namespace ltl
}

template <typename T> constexpr auto is_derived_from(type_t<T> type) {
  return ltl::overloader{[type](auto &&x) constexpr->std::enable_if_t<
      IsType<decltype(x)>, decltype(is_base_of(type, x))>{(void)type;
  return {};
}
, [type](auto &&x) constexpr
  -> std::enable_if_t<!IsType<std::decay_t<decltype(x)>>,
                      decltype(is_base_of(type, decay_from(x)))> {
  (void)type;
  return {};
}
}
;
}

template <typename... Ts> constexpr auto invoke_result(type_t<Ts>...) noexcept {
  return type_v<::std::invoke_result_t<Ts...>>;
}

LTL_MAKE_IS_KIND(::std::optional, is_optional, IsOptional, typename);

template <typename ResultType, typename T>
constexpr auto copy_cv_reference(type_t<T> type) {
  if_constexpr(is_lvalue_reference(type)) {
    if_constexpr(is_const(remove_reference(type))) {
      return type_v<const ResultType &>;
    }
    else {
      return type_v<ResultType &>;
    }
  }

  else_if_constexpr(is_rvalue_reference(type)) { return type_v<ResultType &&>; }

  else {
    return type_v<ResultType>;
  }
}

} // namespace ltl
