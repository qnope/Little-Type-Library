#pragma once
#include "bool_t.h"
#include <type_traits>

namespace ltl {
template <typename T> struct type_t {
  using type = T;
  constexpr type_t() = default;
};

template <typename T> constexpr type_t<T> type_v{};

template <typename T1, typename T2>
[[nodiscard]] constexpr false_t operator==(type_t<T1>, type_t<T2>) {
  return {};
}

template <typename T>
[[nodiscard]] constexpr true_t operator==(type_t<T>, type_t<T>) {
  return {};
}

template <typename T1, typename T2>
[[nodiscard]] constexpr true_t operator!=(type_t<T1>, type_t<T2>) {
  return {};
}

template <typename T>
[[nodiscard]] constexpr false_t operator!=(type_t<T>, type_t<T>) {
  return {};
}

} // namespace ltl

#define decltype_t(t) typename std::decay_t<decltype(t)>::type

#define typed_static_assert(f) static_assert(decltype(f){});
#define if_constexpr(c) if constexpr (decltype(c){})

#define LTL_REQUIRE(b) typename = std::enable_if_t<(b)>
#define LTL_REQUIRE_T(b) std::enable_if_t<decltype(b){}, bool> = true
#define LTL_REQUIRE_RT(b, returnType) std::enable_if_t<(b), returnType>
#define LTL_REQUIRE_RE(b, returnExpression)                                    \
  std::enable_if_t<(b), decltype(returnExpression)>
