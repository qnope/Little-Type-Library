#pragma once
#include "../number_t.h"
#include "../type_t.h"
#include "tuple_t.h"

namespace ltl {
// Convenience types
template <typename... Types> using type_list_t = tuple_t<type_t<Types>...>;
template <size_t... Ns> using number_list_t = tuple_t<number_t<Ns>...>;
template <bool... Bs> using bool_list_t = tuple_t<bool_t<Bs>...>;

// Convenience variables
template <typename... Types> constexpr type_list_t<Types...> type_list_v{};
template <size_t... Ns> constexpr number_list_t<Ns...> number_list_v{};
template <bool... Bs> constexpr bool_list_t<Bs...> bool_list_v{};

// Conditional functions
template <typename T>[[nodiscard]] constexpr false_t is_tuple_t(T) {
  return {};
}
template <typename... Ts>
[[nodiscard]] constexpr true_t is_tuple_t(tuple_t<Ts...>) {
  return {};
}

template <typename T>[[nodiscard]] constexpr false_t is_type_list_t(T) {
  return {};
}
template <typename... Ts>
[[nodiscard]] constexpr true_t is_type_list_t(type_list_t<Ts...>) {
  return {};
}

template <typename T>[[nodiscard]] constexpr false_t is_number_list_t(T) {
  return {};
}
template <size_t... Ns>
[[nodiscard]] constexpr true_t is_number_list_t(number_list_t<Ns...>) {
  return {};
}

template <typename T>[[nodiscard]] constexpr false_t is_bool_list_t(T) {
  return {};
}
template <bool... Bs>
[[nodiscard]] constexpr true_t is_bool_list_t(bool_list_t<Bs...>) {
  return {};
}

template <typename T>[[nodiscard]] constexpr false_t is_number_t(T) {
  return {};
}
template <size_t N>[[nodiscard]] constexpr true_t is_number_t(number_t<N>) {
  return {};
}

template <typename T>[[nodiscard]] constexpr false_t is_bool_t(T) { return {}; }
template <bool B>[[nodiscard]] constexpr true_t is_bool_t(bool_t<B>) {
  return {};
}

#define IS_TYPE(name)                                                          \
  template <typename T>                                                        \
  [[nodiscard]] constexpr decltype(name(std::declval<T>())) name(type_t<T>) {  \
    return {};                                                                 \
  }

IS_TYPE(is_bool_t)
IS_TYPE(is_bool_list_t)
IS_TYPE(is_number_t)
IS_TYPE(is_number_list_t)
IS_TYPE(is_type_list_t)
IS_TYPE(is_tuple_t)
#undef IS_TYPE
} // namespace ltl
