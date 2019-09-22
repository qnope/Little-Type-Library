#pragma once

#include "Tuple.h"

namespace ltl {
// Conditional functions
LTL_MAKE_IS_KIND(tuple_t, is_tuple_t, IsTuple, is_tuple_lifted, typename);
LTL_MAKE_IS_KIND(type_list_t, is_type_list_t, IsTypeList, is_type_list_lifted, typename);

LTL_MAKE_IS_KIND(number_list_t, is_number_list_t, IsNumberList, is_number_list_lifted,
                 int);
LTL_MAKE_IS_KIND(bool_list_t, is_bool_list_t, IsBoolList, is_bool_list_lifted, bool);

template <typename F, typename Tuple>
constexpr decltype(auto) apply(Tuple &&tuple,
                               F &&f) noexcept(noexcept(FWD(tuple)(FWD(f)))) {
  typed_static_assert(is_tuple_t(tuple));
  return FWD(tuple)(FWD(f));
}

template <typename F, typename Tuple> F for_each(Tuple &&tuple, F &&f) {
  typed_static_assert(is_tuple_t(tuple));

  auto retrieveAllArgs = [&f](auto &&... xs) { (FWD(f)(FWD(xs)), ...); };
  ltl::apply(FWD(tuple), retrieveAllArgs);
  return FWD(f);
}

////////////////////// Algorithm tuple
template <typename N1, typename N2>
[[nodiscard]] constexpr auto build_index_sequence(N1 n1, N2 n2) {
  return tuple_t<>::build_index_sequence(n1, n2);
}

template <typename N>[[nodiscard]] constexpr auto build_index_sequence(N n) {
  return tuple_t<>::build_index_sequence(0_n, n); // does not compile
}

template <typename... Ts, typename T>
constexpr auto contains_type(const tuple_t<Ts...> &tuple, type_t<T> type) {
  if_constexpr(is_type_list_t(tuple)) return (false_v || ... || (Ts{} == type));
  else return contains_type(type_list_v<Ts...>, type);
}

template <typename... Ts, typename T>
constexpr auto count_type(const tuple_t<Ts...> &tuple, type_t<T> type) {
  if_constexpr(is_type_list_t(tuple)) return (0_n + ... + bool_to_number(Ts{} == type));
  else return count_type(type_list_v<Ts...>, type);
}

template <typename... Ts, typename T, int N = 0>
constexpr auto find_type(const tuple_t<Ts...> &tuple, type_t<T> type,
                         number_t<N> first = {}) {
  if_constexpr(is_type_list_t(tuple)) {
    if_constexpr(tuple[first] == type) return first;
    else return find_type(tuple, type, first + 1_n);
  }

  else return find_type(type_list_v<Ts...>, type, first);
}

template <typename... Ts, typename P, int N = 0>
constexpr auto find_if_type(const tuple_t<Ts...> &tuple, P &&p, number_t<N> first = {}) {
  if_constexpr(is_type_list_t(tuple)) {
    if_constexpr(FWD(p)(tuple[first])) return first;
    else return find_if_type(tuple, FWD(p), first + 1_n);
  }
  else return find_if_type(type_list_v<Ts...>, FWD(p), first);
}

template <typename... Ts, typename P>
constexpr auto contains_if_type(const tuple_t<Ts...> &tuple, P &&p) {
  if_constexpr(is_type_list_t(tuple)) return (false_v || ... || (FWD(p)(Ts{})));
  else return contains_type(type_list_v<Ts...>, FWD(p));
}

template <typename... Ts, typename P>
constexpr auto count_if_type(const tuple_t<Ts...> &tuple, P &&p) {
  if_constexpr(is_type_list_t(tuple)) return (0_n + ... + bool_to_number(FWD(p)(Ts{})));
  else return count_type(type_list_v<Ts...>, FWD(p));
}

template <typename... Ts, typename P>
constexpr auto all_of_type(const tuple_t<Ts...> &tuple, P &&p) {
  if_constexpr(is_type_list_t(tuple)) return (true_v && ... && (FWD(p)(Ts{})));
  else return all_of_type(type_list_v<Ts...>, FWD(p));
}

template <typename... Ts, typename P>
constexpr auto any_of_type(const tuple_t<Ts...> &tuple, P &&p) {
  if_constexpr(is_type_list_t(tuple)) return (false_v || ... || (FWD(p)(Ts{})));
  else return any_of_type(type_list_v<Ts...>, FWD(p));
}

template <typename... Ts, typename P>
constexpr auto none_of_type(const tuple_t<Ts...> &tuple, P &&p) {
  return !any_of_type(tuple, FWD(p));
}

template <template <typename...> typename T, typename List>
struct build_from_type_listImpl;

template <template <typename...> typename T, typename... Ts>
struct build_from_type_listImpl<T, type_list_t<Ts...>> {
  using type = T<Ts...>;
};

template <template <typename...> typename T, typename Ts>
using build_from_type_list = typename build_from_type_listImpl<T, std::decay_t<Ts>>::type;

} // namespace ltl
