#pragma once

#include <iterator>
#include <optional>

#include "ltl.h"
#include "traits.h"

namespace ltl {
LTL_MAKE_IS_KIND(::std::optional, is_optional, IsOptional, typename);

template <typename T, bool b> using requires_t = std::enable_if_t<b, T>;
template <bool b> using requires_void = requires_t<void, b>;

template <typename T>
constexpr bool IsIterable = decltype(is_iterable(std::declval<T>()))::value;

template <typename T>
constexpr bool IsConst = decltype(is_const(std::declval<T>()))::value;

template <typename T>
constexpr bool IsFloatingPoint =
    decltype(is_floating_point(std::declval<T>()))::value;

template <typename T>
constexpr bool IsIntegral = decltype(is_integral(std::declval<T>()))::value;

template <typename T, typename... Ts>
constexpr bool IsSame = (true_v && ... && (ltl::type_v<T> == ltl::type_v<Ts>));

template <typename T>
constexpr bool IsRValueReference = std::is_rvalue_reference_v<T &&>;

} // namespace ltl

#define requires_f(x) ::ltl::requires_t<bool, (x)> = true
