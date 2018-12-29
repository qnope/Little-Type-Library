#pragma once
#include "template.h"
#include "tuple_t.h"

namespace ltl {
template <typename F, typename Tuple, LTL_REQUIRE_T(is_tuple_t(type_v<Tuple>))>
constexpr decltype(auto) apply(Tuple &&tuple, F &&f) noexcept(
    noexcept(std::forward<Tuple>(tuple)(std::forward<F>(f)))) {
  typed_static_assert(is_tuple_t(tuple));
  return std::forward<Tuple>(tuple)(std::forward<F>(f));
}

template <typename F, typename Tuple, LTL_REQUIRE_T(is_tuple_t(type_v<Tuple>))>
F &&for_each(Tuple &&tuple, F &&f) {
  typed_static_assert(is_tuple_t(tuple));

  auto retrieveAllArgs = [&f](auto &&... xs) {
    (std::forward<F>(f)(std::forward<decltype(xs)>(xs)), ...);
  };
  ltl::apply(std::forward<Tuple>(tuple), retrieveAllArgs);
  return std::forward<F>(f);
}
} // namespace ltl
