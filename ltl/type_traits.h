#pragma once
#include <type_traits>

#include "../lpl/lpl.h"
#include "is_valid.h"
#include "tuple.h"
#include "type_t.h"

namespace ltl {
#define TRAIT(name)                                                            \
  template <typename T>                                                        \
      [[nodiscard]] constexpr bool_t<std::LPL_CAT(name, _v) < T>>              \
      name(type_t<T>) {                                                        \
    return {};                                                                 \
  }

#define TRAIT_ARG(name)                                                        \
  template <typename T, typename Arg>                                          \
      [[nodiscard]] constexpr bool_t<std::LPL_CAT(name, _v) < T, Arg>>         \
      name(type_t<T>, type_t<Arg>) {                                           \
    return {};                                                                 \
  }                                                                            \
                                                                               \
  template <typename T, typename Arg>                                          \
      [[nodiscard]] constexpr bool_t<std::LPL_CAT(name, _v) < T, Arg>>         \
      name(type_t<T>, type_list_t<Arg>) {                                      \
    return {};                                                                 \
  }

#define TRAIT_ARGS(name)                                                       \
  template <typename T, typename... Args>                                      \
      [[nodiscard]] constexpr bool_t<std::LPL_CAT(name, _v) < T, Args...>>     \
      name(type_t<T>, type_t<Args>...) {                                       \
    return {};                                                                 \
  }                                                                            \
                                                                               \
  template <typename T, typename... Args>                                      \
      [[nodiscard]] constexpr bool_t<std::LPL_CAT(name, _v) < T, Args...>>     \
      name(type_t<T>, type_list_t<Args...>) {                                  \
    return {};                                                                 \
  }

// Primary type categories
LPL_MAP(TRAIT, is_void, is_null_pointer, is_integral, is_floating_point,
        is_array, is_enum, is_union, is_class, is_function, is_pointer,
        is_lvalue_reference, is_rvalue_reference, is_member_object_pointer,
        is_member_function_pointer)

// Composite type categories
LPL_MAP(TRAIT, is_fundamental, is_arithmetic, is_scalar, is_object, is_compound,
        is_reference, is_member_pointer)

// Type properties
LPL_MAP(TRAIT, is_const, is_volatile, is_trivial, is_trivially_copyable,
        is_standard_layout, has_unique_object_representations, is_empty,
        is_polymorphic, is_abstract, is_final, is_aggregate, is_signed,
        is_unsigned)

// Supported operations
LPL_MAP(TRAIT, is_default_constructible, is_trivially_default_constructible,
        is_nothrow_default_constructible, is_copy_constructible,
        is_trivially_copy_constructible, is_nothrow_copy_constructible,
        is_move_constructible, is_trivially_move_constructible,
        is_nothrow_move_constructible)

LPL_MAP(TRAIT, is_copy_assignable, is_trivially_copy_assignable,
        is_nothrow_copy_assignable, is_move_assignable,
        is_trivially_move_assignable, is_nothrow_move_assignable)

LPL_MAP(TRAIT_ARG, is_assignable, is_trivially_assignable,
        is_nothrow_assignable, is_swappable_with, is_nothrow_swappable_with)

LPL_MAP(TRAIT, is_destructible, is_trivially_destructible,
        is_nothrow_destructible, has_virtual_destructor, is_swappable,
        is_nothrow_swappable)

LPL_MAP(TRAIT_ARGS, is_constructible, is_trivially_constructible,
        is_nothrow_constructible)

// Type relationships
LPL_MAP(TRAIT_ARG, is_same, is_base_of, is_convertible)

LPL_MAP(TRAIT_ARGS, is_invocable, is_invocable_r, is_nothrow_invocable,
        is_nothrow_invocable_r)

#undef TRAIT
#undef TRAIT_ARG
#undef TRAIT_ARGS

#define TRAIT(name)                                                            \
  template <typename T>                                                        \
      [[nodiscard]] constexpr number_t<std::LPL_CAT(name, _v) < T>>            \
      name(type_t<T>) {                                                        \
    return {};                                                                 \
  }

// Property queries
template <typename T, size_t N = 0>
[[nodiscard]] constexpr number_t<std::extent_v<T, N>>
extent(type_t<T>, number_t<N> = number_t<N>{}) {
  return {};
}

LPL_MAP(TRAIT, alignment_of, rank)

#undef TRAIT

#define TRAIT(name)                                                            \
  template <typename T>                                                        \
      [[nodiscard]] constexpr type_t<std::LPL_CAT(name, _t) < T>>              \
      name(type_t<T>) {                                                        \
    return {};                                                                 \
  }

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

#undef TRAIT

template <typename T>[[nodiscard]] constexpr auto is_iterable(type_t<T>) {
  constexpr auto trait = IS_VALID((x), (std::begin(x), std::end(x)));
  return decltype(trait(std::declval<T>())){};
}

template <typename T>[[nodiscard]] constexpr auto is_iterable(T) {
  return is_iterable(type_v<T>);
}

} // namespace ltl
