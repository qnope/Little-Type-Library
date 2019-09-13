#pragma once
#include "concept.h"
#include "ltl.h"
#include <tuple>

namespace ltl {
template <typename... Ts> class tuple_t {
public:
  constexpr static auto length = number_v<sizeof...(Ts)>;
  constexpr static auto isEmpty = length == 0_n;

  template <bool isNotEmpty = !isEmpty, requires_f(isNotEmpty)>
  constexpr tuple_t() : m_storage{} {}

  constexpr tuple_t(Ts... ts) : m_storage{FWD(ts)...} {}

  template <typename F>
      constexpr decltype(auto) operator()(F &&f) &
      noexcept(noexcept(std::declval<F>()(std::declval<Ts &>()...))) {
    return std::apply(FWD(f), m_storage);
  }

  template <typename F>
  constexpr decltype(auto) operator()(F &&f) const &noexcept(
      noexcept(std::declval<F>()(std::declval<const Ts &>()...))) {
    return std::apply(FWD(f), m_storage);
  }

  template <typename F>
      constexpr decltype(auto) operator()(F &&f) &&
      noexcept(noexcept(std::declval<F>()(std::declval<Ts>()...))) {
    return std::apply(FWD(f), std::move(m_storage));
  }

  template <int N>[[nodiscard]] constexpr auto &get(number_t<N> n) & noexcept {
    typed_static_assert(n < length);
    return std::get<N>(m_storage);
  }

  template <int N>
  [[nodiscard]] constexpr auto &get(number_t<N> n) const &noexcept {
    typed_static_assert(n < length);
    return std::get<N>(m_storage);
  }

  template <int N>
      [[nodiscard]] constexpr auto &&get(number_t<N> n) && noexcept {
    typed_static_assert(n < length);
    return std::get<N>(std::move(m_storage));
  }

  template <int N>[[nodiscard]] constexpr auto &get() & noexcept {
    return get(number_v<N>);
  }

  template <int N>[[nodiscard]] constexpr auto &get() const &noexcept {
    return get(number_v<N>);
  }

  template <int N>[[nodiscard]] constexpr auto &&get() && noexcept {
    return std::move(*this).get(number_v<N>);
  }

  template <int N>
      [[nodiscard]] constexpr auto &operator[](number_t<N> n) & noexcept {
    return get(n);
  }

  template <int N>
  [[nodiscard]] constexpr auto &operator[](number_t<N> n) const &noexcept {
    return get(n);
  }

  template <int N>
      [[nodiscard]] constexpr auto &&operator[](number_t<N> n) && noexcept {
    return std::move(*this).get(n);
  }

  template <int... Is>
  [[nodiscard]] constexpr auto extract(number_t<Is>... ns) const &noexcept {
    constexpr tuple_t<type_t<Ts>...> types{};
    return tuple_t<decltype_t(types[ns])...>{get(ns)...};
  }

  template <int... Is>
      [[nodiscard]] constexpr auto extract(number_t<Is>... ns) && noexcept {
    constexpr tuple_t<type_t<Ts>...> types{};
    return tuple_t<decltype_t(types[ns])...>{std::move(*this).get(ns)...};
  }

#define OP(op)                                                                 \
  [[nodiscard]] constexpr bool operator op(const tuple_t<Ts...> &t)            \
      const noexcept {                                                         \
    return t.m_storage op m_storage;                                           \
  }

  LPL_MAP(OP, ==, !=, <=, <, >, >=)

#undef OP

  template <typename T>
  [[nodiscard]] constexpr auto push_back(T &&newValue) const & {
    auto fwdAll = [&newValue](auto &... xs) {
      return tuple_t<Ts..., decay_reference_wrapper_t<T>>{xs..., FWD(newValue)};
    };
    return std::apply(fwdAll, m_storage);
  }

  template <typename T>[[nodiscard]] constexpr auto push_back(T &&newValue) && {
    auto fwdAll = [&newValue](Ts &&... xs) {
      return tuple_t<Ts..., decay_reference_wrapper_t<T>>{FWD(xs)...,
                                                          FWD(newValue)};
    };
    return std::apply(fwdAll, std::move(m_storage));
  }

  template <typename T>
  [[nodiscard]] constexpr auto push_front(T &&newValue) const & {
    auto fwdAll = [&newValue](auto &... xs) {
      return tuple_t<decay_reference_wrapper_t<T>, Ts...>{FWD(newValue), xs...};
    };
    return std::apply(fwdAll, m_storage);
  }

  template <typename T>
  [[nodiscard]] constexpr auto push_front(T &&newValue) && {
    auto fwdAll = [&newValue](Ts &&... xs) {
      return tuple_t<decay_reference_wrapper_t<T>, Ts...>{FWD(newValue),
                                                          FWD(xs)...};
    };
    return std::apply(fwdAll, std::move(m_storage));
  }

  [[nodiscard]] constexpr auto pop_back() const & {
    auto extracter = [this](auto... numbers) { return extract(numbers...); };
    constexpr auto numbers = build_index_sequence(length - 1_n);
    return numbers(extracter);
  }

  [[nodiscard]] constexpr auto pop_back() && {
    auto extracter = [this](auto... numbers) {
      return std::move(*this).extract(numbers...);
    };
    constexpr auto numbers = build_index_sequence(length - 1_n);
    return numbers(extracter);
  }

  [[nodiscard]] constexpr auto pop_front() const & {
    auto extracter = [this](auto... numbers) {
      return this->extract(numbers...);
    };
    constexpr auto numbers = build_index_sequence(1_n, length);
    return apply(numbers, extracter);
  }

  [[nodiscard]] constexpr auto pop_front() && {
    auto extracter = [this](auto... numbers) {
      return std::move(*this).extract(numbers...);
    };
    constexpr auto numbers = build_index_sequence(1_n, length);
    return apply(numbers, extracter);
  }

  template <typename N1, typename N2>
  [[nodiscard]] static constexpr auto build_index_sequence(N1 n1, N2 n2) {
    return build_index_sequence_helper(n1, n2);
  }

  template <typename N>
  [[nodiscard]] static constexpr auto build_index_sequence(N n) {
    return build_index_sequence(0_n, n);
  }

private:
  template <int N1, int N2, typename List = tuple_t<>>
  [[nodiscard]] static constexpr auto
  build_index_sequence_helper(number_t<N1> n1, number_t<N2> n2,
                              const List list = List{}) {
    typed_static_assert_msg(n1 <= n2, "n1 must be lesser or equal to n2");
    if constexpr (n1 == n2) {
      return list;
    }

    else {
      return build_index_sequence_helper(n1 + 1_n, n2, list.push_back(n1));
    }
  }

private:
  std::tuple<Ts...> m_storage;
};

template <typename... Ts>
tuple_t(Ts...)->tuple_t<decay_reference_wrapper_t<Ts>...>;

////////////////////// Templates
/// Convenience types
template <typename... Types> using type_list_t = tuple_t<type_t<Types>...>;
template <int... Ns> using number_list_t = tuple_t<number_t<Ns>...>;
template <bool... Bs> using bool_list_t = tuple_t<bool_t<Bs>...>;

// Convenience variables
template <typename... Types> constexpr type_list_t<Types...> type_list_v{};
template <int... Ns> constexpr number_list_t<Ns...> number_list_v{};
template <bool... Bs> constexpr bool_list_t<Bs...> bool_list_v{};

// Conditional functions
LTL_MAKE_IS_KIND(tuple_t, is_tuple_t, IsTuple, typename);
LTL_MAKE_IS_KIND(type_list_t, is_type_list_t, IsTypeList, typename);

LTL_MAKE_IS_KIND(number_list_t, is_number_list_t, IsNumberList, int);
LTL_MAKE_IS_KIND(bool_list_t, is_bool_list_t, IsBoolList, bool);

/////////////////////// Arguments
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
  if_constexpr(is_type_list_t(tuple)) return (0_n + ... +
                                              bool_to_number(Ts{} == type));
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
constexpr auto find_if_type(const tuple_t<Ts...> &tuple, P &&p,
                            number_t<N> first = {}) {
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
  if_constexpr(is_type_list_t(tuple)) return (0_n + ... +
                                              bool_to_number(FWD(p)(Ts{})));
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
using build_from_type_list =
    typename build_from_type_listImpl<T, std::decay_t<Ts>>::type;

} // namespace ltl

namespace std {
template <typename... Ts>
struct tuple_size<::ltl::tuple_t<Ts...>>
    : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template <std::size_t I, typename... Ts>
struct tuple_element<I, ::ltl::tuple_t<Ts...>> {
  using type =
      decltype(std::declval<::ltl::tuple_t<Ts...>>().template get<I>());
};

template <std::size_t I, typename... Ts>
struct tuple_element<I, const ::ltl::tuple_t<Ts...>> {
  using type =
      decltype(std::declval<const ::ltl::tuple_t<Ts...>>().template get<I>());
};

template <std::size_t N, typename Tuple, requires_f(::ltl::IsTuple<Tuple>)>
decltype(auto) get(Tuple &&tuple) {
  return FWD(tuple)[number_v<N>];
}

} // namespace std

#define FROM_VARIADIC(args)                                                    \
  ::ltl::tuple_t<decltype(args)...> { args... }

#define TO_VARIADIC(tuple, var, expr) tuple([](auto &&... var) { expr; });
#define TO_VARIADIC_RETURN(tuple, var, expr)                                   \
  tuple([](auto &&... var) { return expr; });
