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

constexpr auto get_tail = [](auto &&, auto &&... ts) {
  return ltl::tuple_t{FWD(ts)...};
};

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
constexpr auto transform_type(Tuple &&tuple, F &&f) {
  typed_static_assert(is_tuple_t(tuple));
  auto build_tuple = [&f](auto &&... xs) {
    return ltl::tuple_t<decltype(std::forward<F>(f)(FWD(xs)))...>{
        std::forward<F>(f)(FWD(xs))...};
  };
  return FWD(tuple)(build_tuple);
}

template <typename Tuple, typename F>
constexpr auto accumulate_type(Tuple &&tuple, F &&f) {
  return FWD(tuple)(FWD(f));
}

template <typename Tuple> constexpr auto accumulate_type(Tuple &&tuple) {
  auto accumulate = [](auto &&... xs) { return (... + FWD(xs)); };
  return accumulate_type(FWD(tuple), accumulate);
}

////////////////////// Algorithm tuple
template <typename... Ts, typename T>
constexpr auto contains_type(const tuple_t<Ts...> &tuple,
                             [[maybe_unused]] type_t<T> type) {
  if_constexpr(is_type_list_t(tuple)) return (false_v || ... || (Ts{} == type));
  else return contains_type(type_list_v<Ts...>, type);
}

template <typename... Ts, typename T>
constexpr auto count_type(const tuple_t<Ts...> &tuple,
                          [[maybe_unused]] type_t<T> type) {
  if_constexpr(is_type_list_t(tuple)) return (0_n + ... +
                                              bool_to_number(Ts{} == type));
  else return count_type(type_list_v<Ts...>, type);
}

template <typename... Ts, typename T, int N = 0>
constexpr auto find_type(const tuple_t<Ts...> &tuple,
                         [[maybe_unused]] type_t<T> type,
                         number_t<N> first = {}) {
  if_constexpr(is_type_list_t(tuple)) {
    if_constexpr(first == tuple.length) return nullopt_type;
    else_if_constexpr(tuple[first] == type) return optional_type{first};
    else return find_type(tuple, type, first + 1_n);
  }

  else return find_type(type_list_v<Ts...>, type, first);
}

template <typename... Ts, typename P, int N = 0>
constexpr auto find_if_type(const tuple_t<Ts...> &tuple, [[maybe_unused]] P p,
                            number_t<N> first = {}) {
  if_constexpr(is_type_list_t(tuple)) {
    if_constexpr(first == tuple.length) return nullopt_type;
    else_if_constexpr(p(tuple[first])) return optional_type{first};
    else return find_if_type(tuple, p, first + 1_n);
  }
  else return find_if_type(type_list_v<Ts...>, p, first);
}

template <typename... Ts, typename P>
constexpr auto contains_if_type(const tuple_t<Ts...> &tuple,
                                [[maybe_unused]] P p) {
  if_constexpr(is_type_list_t(tuple)) return (false_v || ... || p(Ts{}));
  else return contains_if_type(type_list_v<Ts...>, p);
}

template <typename... Ts, typename P>
constexpr auto count_if_type(const tuple_t<Ts...> &tuple,
                             [[maybe_unused]] P p) {
  if_constexpr(is_type_list_t(tuple)) return (0_n + ... +
                                              bool_to_number(p(Ts{})));
  else return count_if_type(type_list_v<Ts...>, p);
}

template <typename... Ts, typename P>
constexpr auto all_of_type(const tuple_t<Ts...> &tuple, [[maybe_unused]] P p) {
  if_constexpr(is_type_list_t(tuple)) return (true_v && ... && (p(Ts{})));
  else return all_of_type(type_list_v<Ts...>, p);
}

template <typename... Ts, typename P>
constexpr auto any_of_type(const tuple_t<Ts...> &tuple, [[maybe_unused]] P p) {
  if_constexpr(is_type_list_t(tuple)) return (false_v || ... || (p(Ts{})));
  else return any_of_type(type_list_v<Ts...>, p);
}

template <typename... Ts, typename P>
constexpr auto none_of_type(const tuple_t<Ts...> &tuple, [[maybe_unused]] P p) {
  return !any_of_type(tuple, p);
}

namespace detail {
template <typename T> struct unique_wrapper_t {};
template <typename T> constexpr unique_wrapper_t<T> unique_wrapper_v{};

template <typename... Ts, typename T>
constexpr auto operator+(type_list_t<Ts...> list, unique_wrapper_t<type_t<T>>) {
  constexpr auto type = type_v<T>;
  if_constexpr(contains_type(list, type)) { //
    return list;
  }
  else {
    return type_list_v<Ts..., T>;
  }
}

} // namespace detail

template <typename... Ts>
constexpr auto unique_type(const tuple_t<Ts...> &tuple) {
  if_constexpr(is_type_list_t(tuple)) { //
    return (type_list_v<> + ... + (detail::unique_wrapper_v<Ts>));
  }
  else { //
    return unique_type(ltl::type_list_v<Ts...>);
  }
}

template <typename... Ts>
constexpr auto is_unique_type(const tuple_t<Ts...> &tuple) {
  if_constexpr(is_type_list_t(tuple)) { //
    return (true_v && ... && (count_type(tuple, Ts{}) == 1_n));
  }
  else { //
    return is_unique_type(ltl::type_list_v<Ts...>);
  }
}

namespace detail {
template <typename T> constexpr auto optional_maker(T t, true_t) {
  return ltl::tuple_t{t};
}

template <typename T> constexpr auto optional_maker(T, false_t) {
  return ltl::tuple_t<>{};
}
} // namespace detail

template <typename... Ts, typename P>
constexpr auto filter_type(const tuple_t<Ts...> &tuple, [[maybe_unused]] P p) {
  if_constexpr(is_type_list_t(tuple)) { //
    return (ltl::type_list_v<> + ... + (detail::optional_maker(Ts{}, p(Ts{}))));
  }
  else {
    return filter_type(ltl::type_list_v<Ts...>, p);
  }
}

template <typename F, typename T, typename... Tuples,
          requires_f(ltl::IsTuple<T>)>
constexpr void zip_with(F &&f, T &&tuple, Tuples &&... tuples) {
  auto indexer = tuple.make_indexer();
  typed_static_assert_msg(((ltl::is_tuple_t(tuples)) && ... && true_v),
                          "All tuples must be tuples");
  typed_static_assert_msg((... && (tuples.length == indexer.length)),
                          "All tuples must be of the same length");
  for_each(indexer, [&f, &tuple, &tuples...](auto indices) {
    std::forward<F>(f)(tuple[indices], tuples[indices]...);
  });
}

template <typename T, typename... Tuples, requires_f(ltl::IsTuple<T>)>
constexpr auto zip_type(T &&tuple, Tuples &&... tuples) {
  auto indexer = tuple.make_indexer();
  typed_static_assert_msg(((ltl::is_tuple_t(tuples)) && ... && true_v),
                          "All tuples must be tuples");
  typed_static_assert_msg((... && (tuples.length == indexer.length)),
                          "All tuples must be of the same length");

  auto get_types_for_index = [&tuple, &tuples...](auto index) {
    return ltl::type_list_v<
        ltl::tuple_t<decltype_t(tuple.getTypes()[index]),
                     decltype_t(tuples.getTypes()[index])...>>;
  };

  auto get_tuple_for_index = [&tuple, &tuples...](auto index) {
    return ltl::tuple_t<decltype_t(tuple.getTypes()[index]),
                        decltype_t(tuples.getTypes()[index])...>{
        std::forward<T>(tuple)[index], std::forward<Tuples>(tuples)[index]...};
  };

  return indexer([get_types_for_index, get_tuple_for_index](auto... indices) {
    auto types = (... + (get_types_for_index(indices)));
    return types([get_tuple_for_index, indices...](auto... types) {
      return ltl::tuple_t<decltype_t(types)...>{
          get_tuple_for_index(indices)...};
    });
  });
}

template <typename F, typename T, requires_f(ltl::IsTuple<T>)>
constexpr void enumerate_with(F &&f, T &&tuple) {
  zip_with(FWD(f), tuple.make_indexer(), FWD(tuple));
}

template <typename T, requires_f(ltl::IsTuple<T>)>
constexpr auto enumerate_type(T &&tuple) {
  return zip_type(tuple.make_indexer(), FWD(tuple));
}

namespace detail {
template <typename F, typename T> struct scanl_wrapper {
  scanl_wrapper(F f, T t) : f{f}, t{t} {}
  F f;
  T t;
};

template <typename F, typename T, typename N>
constexpr auto operator+(scanl_wrapper<F, T> wrapper, N &&newElement) {
  auto last = wrapper.t[wrapper.t.length - 1_n];
  auto newTuple = wrapper.t.push_back(wrapper.f(last, FWD(newElement)));
  return scanl_wrapper{wrapper.f, newTuple};
}
} // namespace detail

template <typename F, typename T, typename Tuple>
constexpr auto scanl([[maybe_unused]] F f, T init, Tuple &&tuple) {
  if_constexpr(tuple.isEmpty) { //
    return ltl::tuple_t<T>{std::move(init)};
  }
  else {
    return FWD(tuple)([f, init = std::move(init)](auto &&... xs) {
      return (detail::scanl_wrapper{f, ltl::tuple_t<T>{std::move(init)}} + ... +
              (FWD(xs)))
          .t;
    });
  }
}

template <typename F, typename Tuple, requires_f(ltl::IsTuple<Tuple>)>
constexpr auto partial_sum(F f, Tuple &&tuple) {
  return scanl(std::move(f), 0_n, std::move(tuple)).pop_front();
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
