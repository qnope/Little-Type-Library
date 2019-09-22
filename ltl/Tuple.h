#pragma once

#include "crtp.h"
#include "ltl.h"

#include <utility>

namespace ltl {
namespace detail {
template <std::size_t I, typename T> struct Value {
  constexpr Value() noexcept : m_value{} {};
  constexpr Value(T &&t) noexcept : m_value{FWD(t)} {}
  constexpr Value(const Value &v) noexcept : m_value{v.m_value} {}
  constexpr Value(Value &&v) noexcept : m_value{FWD(v.m_value)} {}

  template <typename _T, typename = std::enable_if_t<ltl::type_v<std::decay_t<_T>> !=
                                                     ltl::type_v<Value>>>
  constexpr Value(_T &&t) noexcept : m_value{FWD(t)} {}

  constexpr Value &operator=(Value v) noexcept {
    m_value = std::move(v.m_value);
    return *this;
  }

  template <typename _T, typename = std::enable_if_t<ltl::type_v<std::decay_t<_T>> !=
                                                     ltl::type_v<Value>>>
  constexpr Value &operator=(_T &&t) noexcept {
    m_value = FWD(t);
    return *this;
  }

  constexpr auto &&operator[](ltl::number_t<I>) && noexcept { return FWD(m_value); }

  constexpr auto &operator[](ltl::number_t<I>) const &noexcept { return m_value; }

  constexpr auto &operator[](ltl::number_t<I>) & noexcept { return m_value; }

  T m_value;
};

template <typename...> class tuple_t;

template <std::size_t... Is, typename... Ts>
class tuple_t<std::index_sequence<Is...>, Ts...>
    : public Value<Is, Ts>...,
      public Comparable<tuple_t<std::index_sequence<Is...>, Ts...>> {
public:
#ifndef _MSC_VER
  using Value<Is, Ts>::operator[]...; // does not compile with msvc, but gcc
                                      // necessite it, clang works everytime
#endif

  constexpr static auto length = number_v<sizeof...(Ts)>;
  constexpr static auto isEmpty = length == 0_n;

  template <bool isNotEmpty = !isEmpty, typename = std::enable_if_t<isNotEmpty>>
  constexpr tuple_t() noexcept {}

  constexpr tuple_t(Ts... ts) : Value<Is, Ts>{FWD(ts)}... {}

  template <typename... _Ts> tuple_t &operator=(const tuple_t<_Ts...> &t) {
    (((*this)[number_v<Is>] = t[number_v<Is>]), ...);
    return *this;
  }

  template <typename... _Ts> tuple_t &operator=(tuple_t<_Ts...> &&t) {
    (((*this)[number_v<Is>] = std::move(t)[number_v<Is>]), ...);
    return *this;
  }

  template <typename F>
      constexpr decltype(auto) operator()(F &&f) &
      noexcept(noexcept(std::declval<F>()(std::declval<Ts &>()...))) {
    return FWD(f)((*this)[number_v<Is>]...);
  }

  template <typename F>
  constexpr decltype(auto) operator()(F &&f) const &noexcept(
      noexcept(std::declval<F>()(std::declval<const Ts &>()...))) {
    return FWD(f)((*this)[number_v<Is>]...);
  }

  template <typename F>
      constexpr decltype(auto) operator()(F &&f) &&
      noexcept(noexcept(std::declval<F>()(std::declval<Ts>()...))) {
    return FWD(f)(std::move(*this)[number_v<Is>]...);
  }

  template <int N>[[nodiscard]] constexpr auto &get(number_t<N> n) & noexcept {
    typed_static_assert(n < length);
    return (*this)[n];
  }

  template <int N>[[nodiscard]] constexpr auto &get(number_t<N> n) const &noexcept {
    typed_static_assert(n < length);
    return (*this)[n];
  }

  template <int N>[[nodiscard]] constexpr auto &&get(number_t<N> n) && noexcept {
    typed_static_assert(n < length);
    return std::move((*this)[n]);
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

  template <typename... _Ts>
  constexpr bool operator==(const tuple_t<_Ts...> &t) const noexcept {
    return (((*this)[number_v<Is>] == t[number_v<Is>]) && ... && true);
  }

  template <typename... _Ts>
  constexpr bool operator<(const tuple_t<_Ts...> &t) const noexcept {
    bool resultComparison = false;
    auto tester = [&resultComparison](const auto &a, const auto &b) {
      if (a > b) {
        resultComparison = false;
        return false;
      }

      else if (a < b) {
        resultComparison = true;
        return false;
      }
      return true;
    };
    ((tester((*this)[number_v<Is>], t[number_v<Is>])) && ...);
    return resultComparison;
  }
};

} // namespace detail

template <typename... Ts>
class tuple_t : public detail::tuple_t<std::index_sequence_for<Ts...>, Ts...> {
public:
  using super = detail::tuple_t<std::index_sequence_for<Ts...>, Ts...>;

  using super::length;
  using super::super;
  using super::operator=;

  constexpr static detail::tuple_t<std::index_sequence_for<Ts...>, type_t<Ts>...> types{};

  template <int... Is>
  [[nodiscard]] constexpr auto extract(number_t<Is>... ns) const &noexcept {
    return tuple_t<decltype_t(types[ns])...>{this->get(ns)...};
  }

  template <int... Is>
      [[nodiscard]] constexpr auto extract(number_t<Is>... ns) && noexcept {
    return tuple_t<decltype_t(types[ns])...>{std::move(*this).get(ns)...};
  }

  template <typename T>[[nodiscard]] constexpr auto push_back(T &&newValue) const & {
    auto fwdAll = [&newValue](auto &... xs) {
      return tuple_t<Ts..., decay_reference_wrapper_t<T>>{xs..., FWD(newValue)};
    };

    return (*this)(fwdAll);
  }

  template <typename T>[[nodiscard]] constexpr auto push_back(T &&newValue) && {
    auto fwdAll = [&newValue](Ts &&... xs) {
      return tuple_t<Ts..., decay_reference_wrapper_t<T>>{FWD(xs)..., FWD(newValue)};
    };
    return std::move(*this)(fwdAll);
  }

  template <typename T>[[nodiscard]] constexpr auto push_front(T &&newValue) const & {
    auto fwdAll = [&newValue](auto &... xs) {
      return tuple_t<decay_reference_wrapper_t<T>, Ts...>{FWD(newValue), xs...};
    };
    return (*this)(fwdAll);
  }

  template <typename T>[[nodiscard]] constexpr auto push_front(T &&newValue) && {
    auto fwdAll = [&newValue](Ts &&... xs) {
      return tuple_t<decay_reference_wrapper_t<T>, Ts...>{FWD(newValue), FWD(xs)...};
    };
    return std::move(*this)(fwdAll);
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
    auto extracter = [this](auto... numbers) { return this->extract(numbers...); };
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

  template <typename N>[[nodiscard]] static constexpr auto build_index_sequence(N n) {
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
};

template <typename... Ts> tuple_t(Ts...)->tuple_t<decay_reference_wrapper_t<Ts>...>;

////////////////////// Templates
/// Convenience types
template <typename... Types> using type_list_t = tuple_t<type_t<Types>...>;
template <int... Ns> using number_list_t = tuple_t<number_t<Ns>...>;
template <bool... Bs> using bool_list_t = tuple_t<bool_t<Bs>...>;

// Convenience variables
template <typename... Types> constexpr type_list_t<Types...> type_list_v{};
template <int... Ns> constexpr number_list_t<Ns...> number_list_v{};
template <bool... Bs> constexpr bool_list_t<Bs...> bool_list_v{};

template <typename... Ts> tuple_t<Ts &...> tie(Ts &... ts) noexcept { return {ts...}; }

} // namespace ltl

namespace std {
template <typename... Ts>
struct tuple_size<::ltl::tuple_t<Ts...>>
    : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template <std::size_t I, typename... Ts> struct tuple_element<I, ::ltl::tuple_t<Ts...>> {
  using type = decltype(std::declval<::ltl::tuple_t<Ts...>>().template get<I>());
};

template <std::size_t I, typename... Ts>
struct tuple_element<I, const ::ltl::tuple_t<Ts...>> {
  using type = decltype(std::declval<const ::ltl::tuple_t<Ts...>>().template get<I>());
};

template <std::size_t N, typename Tuple> decltype(auto) get(Tuple &&tuple) {
  return FWD(tuple)[number_v<N>];
}

} // namespace std

#define FROM_VARIADIC(args)                                                              \
  ::ltl::tuple_t<decltype(args)...> { args... }

#define TO_VARIADIC(tuple, var, expr) tuple([](auto &&... var) { expr; });
#define TO_VARIADIC_RETURN(tuple, var, expr) tuple([](auto &&... var) { return expr; });
