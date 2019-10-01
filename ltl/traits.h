#pragma once

#include "ltl.h"

#include <any>

namespace ltl {
// Uniformize declval
template <typename T> std::add_rvalue_reference_t<T> declval(type_t<T>);
template <typename T> std::add_rvalue_reference_t<T> declval(T &&);

#define TRAIT(name)                                                            \
  constexpr auto LPL_CAT(name) = [](auto &&... xs) constexpr noexcept {        \
    return bool_v<std::LPL_CAT(name, _v) <                                     \
                  std::decay_t<decltype(declval(FWD(xs)))>...>> ;              \
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
  constexpr auto name = [](auto &&x) constexpr noexcept {                      \
    return bool_v<std::LPL_CAT(name, _v) < decltype(declval(FWD(x)))>> ;       \
  };

#define TRAIT_CVNESS(name)                                                     \
  constexpr auto name = [](auto &&x) constexpr {                               \
    return bool_v<std::LPL_CAT(name, _v) <                                     \
                  std::remove_reference_t<decltype(declval(FWD(x)))>>> ;       \
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
  constexpr auto name = [](auto &&x) constexpr noexcept {                      \
    return type_v<std::LPL_CAT(name, _t) < decltype(declval(x))>> ;            \
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

namespace detail {
template <typename F, typename... Args>
constexpr auto is_validImpl(Args &&... args)
    -> decltype(std::declval<F>()(FWD(args)...), true_t{});

template <typename F> constexpr false_t is_validImpl(...);
} // namespace detail

template <typename F> constexpr auto is_valid(F &&) {
  return [](auto &&... xs) {
    return decltype(detail::is_validImpl<F>(declval(FWD(xs))...)){};
  };
}

#define LTL_WRITE_AUTO_WITH_COMMA_IMPL(x) , auto &&x

#define LTL_WRITE_AUTO_IMPL(x, ...)                                            \
  (auto &&x LPL_MAP(LTL_WRITE_AUTO_WITH_COMMA_IMPL, __VA_ARGS__))

#define LTL_ENSURE_NOT_EMPTY(...) (__VA_ARGS__, ())
#define IS_VALID(variables, ...)                                               \
  ltl::is_valid(                                                               \
      [] LPL_IDENTITY(LTL_WRITE_AUTO_IMPL LTL_ENSURE_NOT_EMPTY variables)      \
          -> decltype(__VA_ARGS__, void()) {})

LTL_MAKE_IS_KIND(number_t, is_number_t, IsNumber, int);
LTL_MAKE_IS_KIND(bool_t, is_bool_t, IsBool, bool);

template <typename T> constexpr auto is_type(type_t<T> type) {
  return [type](auto &&x) { return decay_from(declval(FWD(x))) == type; };
}

template <typename T> constexpr auto is_derived_from(type_t<T> type) {
  return [type](auto &&x) {
    return is_base_of(type, decay_from(declval(FWD(x))));
  };
}

template <typename F, typename... Ts>
constexpr auto invoke_result(type_t<F>, type_t<Ts>...) noexcept {
  return type_v<::std::invoke_result_t<F, Ts...>>;
}

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

using std::begin;
using std::end;

constexpr auto is_iterable = IS_VALID((x), begin(x), end(x));
constexpr auto is_generic_callable = IS_VALID((x), x(std::any{}));
} // namespace ltl
