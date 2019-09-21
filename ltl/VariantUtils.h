#pragma once

#include <variant>

#include "ltl.h"

namespace ltl {
template <typename V, typename... Fs> decltype(auto) match(V &&v, Fs &&... fs) {
  return ::std::visit(overloader{FWD(fs)...}, FWD(v));
}

namespace detail {
template <typename... Types>
constexpr auto getVariantTypes(type_t<::std::variant<Types...>>) {
  return type_list_v<Types...>;
}
} // namespace detail

template <typename T> using decltype_template = typename ::std::decay_t<T>::type;

template <typename V, typename... Fs> auto match_result(V &&v, Fs &&... fs) {
  constexpr auto variant_type = type_from(v);
  constexpr auto types = detail::getVariantTypes(decay(variant_type));
  constexpr auto qualified_types = types([variant_type](auto... t) {
    return tuple_t{copy_cv_reference<decltype_t(t)>(variant_type)...};
  });
  overloader function{FWD(fs)...};
  constexpr auto functionType = type_from(function);
  constexpr auto result_types = qualified_types([functionType](auto... t) {
    return ltl::tuple_t{invoke_result(functionType, t)...};
  });
  using result_type = build_from_type_list<std::variant, decltype(result_types)>;
  return std::visit([&function](auto &&x) -> result_type { return function(FWD(x)); },
                    FWD(v));
}

} // namespace ltl
