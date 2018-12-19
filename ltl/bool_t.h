#pragma once
#include "../lpl/lpl.h"

namespace ltl {
template <bool v> struct bool_t {
  static constexpr auto value = v;
  constexpr operator bool() const { return v; }
};

using false_t = bool_t<false>;
using true_t = bool_t<true>;

template <bool v> constexpr bool_t<v> bool_v{};

#define OP(op)                                                                 \
  template <bool v1, bool v2>                                                  \
  [[nodiscard]] constexpr bool_t<v1 op v2> operator op(bool_t<v1>,             \
                                                       bool_t<v2>) {           \
    return {};                                                                 \
  }

LPL_MAP(OP, ==, !=, &&, ||)

#undef OP

template <bool v>[[nodiscard]] constexpr bool_t<!v> operator!(bool_t<v>) {
  return {};
}

} // namespace ltl

constexpr ltl::false_t false_v;
constexpr ltl::true_t true_v;
