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
template <typename T> constexpr false_t is_tuple_t(const T &) { return {}; }
template <typename... Ts> constexpr true_t is_tuple_t(const tuple_t<Ts...> &) {
  return {};
}

template <typename T> constexpr false_t is_type_list_t(const T &) { return {}; }
template <typename... Ts>
constexpr true_t is_type_list_t(const type_list_t<Ts...> &) {
  return {};
}

template <typename T> constexpr false_t is_number_list_t(const T &) {
  return {};
}
template <size_t... Ns>
constexpr true_t is_number_list_t(number_list_t<Ns...>) {
  return {};
}

template <typename T> constexpr false_t is_bool_list_t(const T &) { return {}; }
template <bool... Bs> constexpr true_t is_bool_list_t(bool_list_t<Bs...>) {
  return {};
}

template <typename T> constexpr false_t is_number_t(const T &) { return {}; }
template <size_t N> constexpr true_t is_number_t(const number_t<N> &) {
  return {};
}

template <typename T> constexpr false_t is_bool_t(const T &) { return {}; }
template <bool B> constexpr true_t is_bool_t(const bool_t<B> &) { return {}; }

} // namespace ltl
