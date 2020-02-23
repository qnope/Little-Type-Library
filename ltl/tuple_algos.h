#pragma once

#include "Tuple.h"
#include "optional_type.h"

#define types_from(x) ::ltl::getQualifiedTypeList(type_from(x))

namespace ltl {
template <template <typename... Types> typename List, typename... Types>
constexpr auto getQualifiedTypeList(type_t<const List<Types...> &>) {
  return type_list_v<std::add_lvalue_reference_t<std::add_const_t<Types>>...>;
}

template <template <typename... Types> typename List, typename... Types>
constexpr auto getQualifiedTypeList(type_t<List<Types...> &>) {
  return type_list_v<std::add_lvalue_reference_t<Types>...>;
}

template <template <typename... Types> typename List, typename... Types>
constexpr auto getQualifiedTypeList(type_t<List<Types...> &&>) {
  return type_list_v<std::add_rvalue_reference_t<Types>...>;
}

template <typename F, typename Tuple>
constexpr decltype(auto) apply(Tuple &&tuple,
                               F &&f) noexcept(noexcept(FWD(tuple)(FWD(f)))) {
  typed_static_assert(is_tuple_t(tuple));
  return FWD(tuple)(FWD(f));
}

template <typename F, typename Tuple, requires_f(IsTuple<Tuple>)>
F for_each(Tuple &&tuple, F &&f) {
  typed_static_assert(is_tuple_t(tuple));

  auto retrieveAllArgs = [&f](auto &&... xs) {
    (std::forward<F>(f)(FWD(xs)), ...);
  };
  ltl::apply(FWD(tuple), retrieveAllArgs);
  return FWD(f);
}

template <typename F, typename Tuple>
constexpr decltype(auto) transform(Tuple &&tuple, F &&f) {
  typed_static_assert(is_tuple_t(tuple));
  auto build_tuple = [&f](auto &&... xs) {
    return ltl::tuple_t<decltype(std::forward<F>(f)(FWD(xs)))...>{
        std::forward<F>(f)(FWD(xs))...};
  };
  return FWD(tuple)(build_tuple);
}

////////////////////// Algorithm tuple
template <typename... Ts, typename T>
constexpr auto contains_type(const tuple_t<Ts...> &tuple, type_t<T> type) {
  if_constexpr(is_type_list_t(tuple)) return (false_v || ... || (Ts{} == type));
  else return contains_type(type_list_v<Ts...>, type);
}

template <typename... Ts, typename T>
constexpr auto count_type(const tuple_t<Ts...> &tuple, type_t<T> type) {
  if_constexpr(is_type_list_t(tuple)) return (0_n + ... +
                                              bool_to_number(Ts{} == type));
  else return count_type(type_list_v<Ts...>, type);
}

template <typename... Ts, typename T, int N = 0>
constexpr auto find_type(const tuple_t<Ts...> &tuple, type_t<T> type,
                         number_t<N> first = {}) {
  if_constexpr(is_type_list_t(tuple)) {
    if_constexpr(first == tuple.length) return nullopt_type;
    else_if_constexpr(tuple[first] == type) return optional_type{first};
    else return find_type(tuple, type, first + 1_n);
  }

  else return find_type(type_list_v<Ts...>, type, first);
}

template <typename... Ts, typename P, int N = 0>
constexpr auto find_if_type(const tuple_t<Ts...> &tuple, P p,
                            number_t<N> first = {}) {
  if_constexpr(is_type_list_t(tuple)) {
    if_constexpr(first == tuple.length) return nullopt_type;
    else_if_constexpr(p(tuple[first])) return optional_type{first};
    else return find_if_type(tuple, p, first + 1_n);
  }
  else return find_if_type(type_list_v<Ts...>, p, first);
}

template <typename... Ts, typename P>
constexpr auto contains_if_type(const tuple_t<Ts...> &tuple, P p) {
  if_constexpr(is_type_list_t(tuple)) return (false_v || ... || p(Ts{}));
  else return contains_if_type(type_list_v<Ts...>, p);
}

template <typename... Ts, typename P>
constexpr auto count_if_type(const tuple_t<Ts...> &tuple, P p) {
  if_constexpr(is_type_list_t(tuple)) return (0_n + ... +
                                              bool_to_number(p(Ts{})));
  else return count_if_type(type_list_v<Ts...>, p);
}

template <typename... Ts, typename P>
constexpr auto all_of_type(const tuple_t<Ts...> &tuple, P p) {
  if_constexpr(is_type_list_t(tuple)) return (true_v && ... && (p(Ts{})));
  else return all_of_type(type_list_v<Ts...>, p);
}

template <typename... Ts, typename P>
constexpr auto any_of_type(const tuple_t<Ts...> &tuple, P p) {
  if_constexpr(is_type_list_t(tuple)) return (false_v || ... || (p(Ts{})));
  else return any_of_type(type_list_v<Ts...>, p);
}

template <typename... Ts, typename P>
constexpr auto none_of_type(const tuple_t<Ts...> &tuple, P p) {
  return !any_of_type(tuple, p);
}

template <template <typename...> typename T, typename List>
struct build_from_type_listImpl;

template <template <typename...> typename T, typename... Ts>
struct build_from_type_listImpl<T, type_list_t<Ts...>> {
  using type = T<Ts...>;
};

template <template <typename...> typename T, typename Ts>
using build_from_type_list =
    typename build_from_type_listImpl<T, std::decay_t<Ts>>::type;

} // namespace ltl
