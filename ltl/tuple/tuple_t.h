#pragma once
#include "../decay_reference_wrapper.h"
#include "../number_t.h"
#include "../type_t.h"
#include <tuple>
#include <utility>

namespace ltl {
namespace detail {
template <typename... Ts> struct tuple_applied {
  template <bool isNotEmpty = (sizeof...(Ts) > 0), LTL_REQUIRE(isNotEmpty)>
  constexpr tuple_applied() : m_tuple{Ts{}...} {}

  constexpr tuple_applied(Ts &&... ts) : m_tuple{std::forward<Ts>(ts)...} {}

  template <typename F> decltype(auto) operator()(F &&f) & {
    return std::apply(std::forward<F>(f), m_tuple);
  }

  template <typename F> constexpr decltype(auto) operator()(F &&f) const & {
    return std::apply(std::forward<F>(f), m_tuple);
  }

  template <typename F> constexpr decltype(auto) operator()(F &&f) && {
    return std::apply(std::forward<F>(f), std::move(m_tuple));
  }

  std::tuple<Ts...> m_tuple;
};

} // namespace detail

template <typename... Ts> class tuple_t {
public:
  constexpr static auto length = number_v<sizeof...(Ts)>;
  constexpr static auto isEmpty = length == 0_n;

  template <bool isNotEmpty = !isEmpty, LTL_REQUIRE(isNotEmpty)>
  explicit constexpr tuple_t() : m_storage{} {}

  explicit constexpr tuple_t(Ts... ts) : m_storage{std::forward<Ts>(ts)...} {}

  template <typename F>
      decltype(auto) operator()(F &&f) &
      noexcept(noexcept(std::declval<F>()(std::declval<Ts &>()...))) {
    return m_storage(std::forward<F>(f));
  }

  template <typename F>
  constexpr decltype(auto) operator()(F &&f) const &noexcept(
      noexcept(std::declval<F>()(std::declval<const Ts &>()...))) {
    return m_storage(std::forward<F>(f));
  }

  template <typename F>
      constexpr decltype(auto) operator()(F &&f) &&
      noexcept(noexcept(std::declval<F>()(std::declval<Ts>()...))) {
    return std::move(m_storage)(std::forward<F>(f));
  }

  template <std::size_t N>[[nodiscard]] auto &get(number_t<N> n) & noexcept {
    typed_static_assert(n < length);
    return std::get<N>(m_storage.m_tuple);
  }

  template <std::size_t N>
  [[nodiscard]] constexpr const auto &get(number_t<N> n) const &noexcept {
    typed_static_assert(n < length);
    return std::get<N>(m_storage.m_tuple);
  }

  template <std::size_t N>
      [[nodiscard]] constexpr auto &&get(number_t<N> n) && noexcept {
    typed_static_assert(n < length);
    return std::get<N>(std::move(m_storage.m_tuple));
  }

  template <std::size_t N>
      [[nodiscard]] auto &operator[](number_t<N> n) & noexcept {
    return get(n);
  }

  template <std::size_t N>
  [[nodiscard]] constexpr const auto &
  operator[](number_t<N> n) const &noexcept {
    return get(n);
  }

  template <std::size_t N>
      [[nodiscard]] constexpr auto &&operator[](number_t<N> n) && noexcept {
    return std::move(*this).get(n);
  }

  template <std::size_t... Is>
  [[nodiscard]] constexpr auto extract(number_t<Is>... ns) const &noexcept {
    constexpr tuple_t<type_t<Ts>...> types{};
    return tuple_t<decltype_t(types[ns])...>{get(ns)...};
  }

  template <std::size_t... Is>
      [[nodiscard]] constexpr auto extract(number_t<Is>... ns) && noexcept {
    constexpr tuple_t<type_t<Ts>...> types{};
    return tuple_t<decltype_t(types[ns])...>{std::move(*this).get(ns)...};
  }

  template <typename... _Ts,
            LTL_REQUIRE(type_v<tuple_t<_Ts...>> != type_v<tuple_t<Ts...>>)>
  [[nodiscard]] constexpr bool operator==(const tuple_t<_Ts...> &) const
      noexcept {
    return false;
  }

  template <typename... _Ts,
            LTL_REQUIRE(type_v<tuple_t<_Ts...>> != type_v<tuple_t<Ts...>>)>
  [[nodiscard]] constexpr bool operator!=(const tuple_t<_Ts...> &) const
      noexcept {
    return true;
  }

  [[nodiscard]] constexpr bool operator==(const tuple_t<Ts...> &t) const
      noexcept {
    return t.m_storage.m_tuple == m_storage.m_tuple;
  }

  [[nodiscard]] constexpr bool operator!=(const tuple_t<Ts...> &t) const
      noexcept {
    return t.m_storage.m_tuple != m_storage.m_tuple;
  }

  template <typename T>
  [[nodiscard]] constexpr auto push_back(T &&newValue) const & {
    auto fwdAll = [&newValue](const auto &... xs) {
      return tuple_t<Ts..., decay_reference_wrapper_t<T>>{
          xs..., std::forward<T>(newValue)};
    };
    return m_storage(fwdAll);
  }

  template <typename T>[[nodiscard]] constexpr auto push_back(T &&newValue) && {
    auto fwdAll = [&newValue](Ts &&... xs) {
      return tuple_t<Ts..., decay_reference_wrapper_t<T>>{
          std::forward<Ts>(xs)..., std::forward<T>(newValue)};
    };
    return std::move(m_storage)(fwdAll);
  }

  template <typename T>
  [[nodiscard]] constexpr auto push_front(T &&newValue) const & {
    auto fwdAll = [&newValue](const auto &... xs) {
      return tuple_t<decay_reference_wrapper_t<T>, Ts...>{
          std::forward<T>(newValue), xs...};
    };
    return m_storage(fwdAll);
  }

  template <typename T>
  [[nodiscard]] constexpr auto push_front(T &&newValue) && {
    auto fwdAll = [&newValue](Ts &&... xs) {
      return tuple_t<decay_reference_wrapper_t<T>, Ts...>{
          std::forward<T>(newValue), std::forward<Ts>(xs)...};
    };
    return std::move(m_storage)(fwdAll);
  }

  [[nodiscard]] constexpr auto pop_back() const & {
    auto extracter = [this](auto... numbers) {
      return this->extract(numbers...);
    };
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
    return build_index_sequence_helper(n1, n2, tuple_t<>{});
  }

  template <typename N>
  [[nodiscard]] static constexpr auto build_index_sequence(N n) {
    return build_index_sequence(0_n, n);
  }

private:
  template <typename N1, typename N2, typename List>
  [[nodiscard]] static constexpr auto
  build_index_sequence_helper(N1 n1, N2 n2, const List list) {
    if constexpr (n1 == n2) {
      return list;
    }

    else {
      return build_index_sequence_helper(n1 + 1_n, n2, list.push_back(n1));
    }
  }

private:
  detail::tuple_applied<Ts...> m_storage;
};

template <typename... Ts>
tuple_t(Ts...)->tuple_t<decay_reference_wrapper_t<Ts>...>;

} // namespace ltl
