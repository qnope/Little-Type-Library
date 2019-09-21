#pragma once

#include "ltl.h"
#include <iterator>
#include <optional>

namespace ltl {
LTL_MAKE_IS_KIND(::std::optional, is_optional, IsOptional, typename);

template <typename T>[[nodiscard]] constexpr auto is_iterableImpl(type_t<T>) {
  constexpr auto trait = IS_VALID((x), std::begin(FWD(x)), std::end(FWD(x)));
  return decltype(trait(std::declval<T>())){};
}

template <typename T>[[nodiscard]] constexpr auto is_iterableImpl(T &&x) {
  return is_iterableImpl(type_from(x));
}

constexpr auto is_iterable = [](auto &&x) constexpr { return is_iterableImpl(FWD(x)); };

template <typename T, bool b> using requires_t = std::enable_if_t<b, T>;
template <bool b> using requires_void = requires_t<void, b>;

template <typename T>
constexpr bool IsIterable = decltype(is_iterable(std::declval<T>()))::value;

template <typename T>
constexpr bool IsConst = decltype(is_const(std::declval<T>()))::value;

template <typename T>
constexpr bool IsFloatingPoint = decltype(is_floating_point(std::declval<T>()))::value;

template <typename T>
constexpr bool IsIntegral = decltype(is_integral(std::declval<T>()))::value;

template <typename T, typename... Ts>
constexpr bool IsSame =
    decltype((true_v && ... && (ltl::type_v<T> == ltl::type_v<Ts>)))::value;

} // namespace ltl

#define requires_f(x) ::ltl::requires_t<bool, (x)> = true
