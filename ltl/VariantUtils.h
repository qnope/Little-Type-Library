#pragma once

#include <variant>

#include "traits.h"
#include "tuple_algos.h"

namespace ltl {
template <typename V, typename... Fs> decltype(auto) match(V &&v, Fs &&... fs) {
  return ::std::visit(overloader{FWD(fs)...}, FWD(v));
}

template <typename Variant, typename... Fs>
auto match_result(Variant &&variant, Fs &&... fs) {
  constexpr auto qualified_types = types_from(variant);
  overloader function{FWD(fs)...};
  constexpr auto functionType = type_from(function);
  constexpr auto result_types = qualified_types([functionType](auto... t) {
    return ltl::tuple_t{invoke_result(functionType, t)...};
  });
  using result_type =
      build_from_type_list<std::variant, decltype(result_types)>;
  return std::visit(
      [&function](auto &&x) -> result_type { return function(FWD(x)); },
      FWD(variant));
}

template <typename F, typename Variant>
constexpr auto is_callable_from(F &&f, Variant &&variant) {
  auto is_f_invocable =
      is_valid([&f](auto &&x) -> decltype(FWD(f)(declval(FWD(x))), void()) {});
  constexpr auto qualified_types = types_from(variant);
  return all_of_type(qualified_types, is_f_invocable);
}

} // namespace ltl
