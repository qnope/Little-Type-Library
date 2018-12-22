#pragma once
#include "../lpl/and.h"
#include "bool_t.h"
#include <cstddef>

namespace ltl {
template <std::size_t N> struct number_t {
  constexpr static std::size_t value = N;
};

template <std::size_t N> constexpr number_t<N> number_v{};

namespace detail {
template <char... _digits>
[[nodiscard]] constexpr std::size_t digits_to_size_t() {
  char digits[] = {_digits...};
  std::size_t result = 0;

  for (auto d : digits) {
    result *= 10;
    result += d - '0';
  }

  return result;
}
} // namespace detail

#define OP(op)                                                                 \
  template <std::size_t N1, std::size_t N2>                                    \
  [[nodiscard]] constexpr number_t<N1 op N2> operator op(number_t<N1>,         \
                                                         number_t<N2>) {       \
    return {};                                                                 \
  }

LPL_MAP(OP, +, -, *, /, %)
#undef OP

#define OP(op)                                                                 \
  template <std::size_t N1, std::size_t N2>                                    \
  [[nodiscard]] constexpr bool_t<(N1 op N2)> operator op(number_t<N1>,         \
                                                         number_t<N2>) {       \
    return {};                                                                 \
  }

LPL_MAP(OP, ==, !=, >, >=, <, <=)
#undef OP
} // namespace ltl

template <char... digits>
[[nodiscard]] constexpr ltl::number_t<
    ltl::detail::digits_to_size_t<digits...>()>
operator""_n() {
  return {};
}
