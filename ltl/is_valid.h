#pragma once

#include "../lpl/lpl.h"
#include "bool_t.h"
#include <type_traits>

namespace ltl {
namespace detail {
template <typename F, typename... Fs>
constexpr auto testValidity(F &&f, Fs &&... fs)
    -> decltype(f(std::forward<Fs>(fs)...), true_v);

constexpr false_t testValidity(...);

} // namespace detail

template <typename F> constexpr auto is_valid(F f) {
  return [f](auto &&... x) -> decltype(detail::testValidity(f, x...)) {
    return {};
  };
}

} // namespace ltl

#define LTL_WRITE_AUTO_WITH_COMMA_IMPL(x) , auto x

#define LTL_WRITE_AUTO_IMPL(x, ...)                                            \
  (auto x LPL_MAP(LTL_WRITE_AUTO_WITH_COMMA_IMPL, __VA_ARGS__))

#define LTL_ENSURE_NOT_EMPTY(...) (__VA_ARGS__, ())
#define IS_VALID(variables, expr)                                              \
  ltl::is_valid([] LPL_IDENTITY(LTL_WRITE_AUTO_IMPL LTL_ENSURE_NOT_EMPTY       \
                                    variables) -> decltype(expr, void()) {})
