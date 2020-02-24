#pragma once

#include "Tuple.h"
#include "crtp.h"

namespace ltl {
#define OP(op)                                                                 \
  template <typename T> constexpr auto operator op(T &&t) const noexcept {     \
    return m_values(                                                           \
        [&t](auto &&... xs) { return (true_v && ... && (FWD(xs) op t)); });    \
  }

template <typename... Ts> class AllOf : public Comparable<AllOf<Ts...>> {
  static_assert(sizeof...(Ts) > 0, "Never use AllOf without args");

public:
  constexpr AllOf(Ts &&... ts) noexcept : m_values{FWD(ts)...} {}

  LPL_MAP(OP, <, <=, >, >=, ==, !=)

private:
  ltl::tuple_t<Ts...> m_values;
};

template <typename... Ts> AllOf(Ts &&...)->AllOf<Ts &&...>;

#undef OP

#define OP(op)                                                                 \
  template <typename T> constexpr auto operator op(T &&t) const noexcept {     \
    return m_values(                                                           \
        [&t](auto &&... xs) { return (false_v || ... || (FWD(xs) op t)); });   \
  }

template <typename... Ts> class AnyOf : public Comparable<AnyOf<Ts...>> {
  static_assert(sizeof...(Ts) > 0, "Never use AnyOf without args");

public:
  constexpr AnyOf(Ts &&... ts) noexcept : m_values{FWD(ts)...} {}

  LPL_MAP(OP, <, <=, >, >=, ==, !=)

private:
  ltl::tuple_t<Ts...> m_values;
};

template <typename... Ts> AnyOf(Ts &&...)->AnyOf<Ts &&...>;

#undef OP

#define OP(op)                                                                 \
  template <typename T> constexpr auto operator op(T &&t) const noexcept {     \
    return m_values(                                                           \
        [&t](auto &&... xs) { return !(false_v || ... || (FWD(xs) op t)); });  \
  }

template <typename... Ts> class NoneOf : public Comparable<AllOf<Ts...>> {
  static_assert(sizeof...(Ts) > 0, "Never use NoneOf without args");

public:
  constexpr NoneOf(Ts &&... ts) noexcept : m_values{FWD(ts)...} {}

  LPL_MAP(OP, <, <=, >, >=, ==, !=)

private:
  ltl::tuple_t<Ts...> m_values;
};

template <typename... Ts> NoneOf(Ts &&...)->NoneOf<Ts &&...>;

#undef OP

} // namespace ltl
