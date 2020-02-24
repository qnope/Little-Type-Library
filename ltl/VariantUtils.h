#pragma once

#include <variant>

#include "traits.h"
#include "tuple_algos.h"
#include "functional.h"

namespace ltl {
template <typename V, typename... Fs> decltype(auto) match(V &&v, Fs &&... fs) {
  return ::std::visit(overloader{FWD(fs)...}, FWD(v));
}

template <typename Variant, typename... Fs>
auto match_result(Variant &&variant, Fs ... fs) {
  constexpr auto qualified_types = types_from(variant);
  overloader function{fs...};
  constexpr auto result_from_function = [](auto type) {
    constexpr auto f = type_v<overloader<Fs...>>;
    return invoke_result(f, type);
  };
  using result_types = decltype(transform(qualified_types, result_from_function));

  using result_type =
      build_from_type_list<std::variant, result_types>;
  return std::visit(
      [&function](auto &&x) -> result_type { return function(FWD(x)); },
      FWD(variant));
}

template <typename F, typename Variant>
constexpr auto is_callable_from(F &&, Variant &&variant) {
  constexpr auto is_f_invocable =
      IS_VALID((x), declval(type_v<F>)(declval(FWD(x))));
  constexpr auto qualified_types = types_from(variant);
  return all_of_type(qualified_types, is_f_invocable);
}

} // namespace ltl
