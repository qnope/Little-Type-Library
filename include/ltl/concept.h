#pragma once

#include <iterator>
#include <optional>

#include "ltl.h"
#include "traits.h"

namespace ltl {
LTL_MAKE_IS_KIND(::std::optional, is_optional, IsOptional, typename);

template <typename T, bool b>
using requires_t = std::enable_if_t<b, T>;
template <bool b>
using requires_void = requires_t<void, b>;

template <typename T>
constexpr bool IsIterable = decltype(is_iterable(std::declval<T>()))::value;

template <typename T>
constexpr bool IsConst = std::is_const_v<T>;

template <typename T>
constexpr bool IsFloatingPoint = std::is_floating_point_v<T>;

template <typename T>
constexpr bool IsIntegral = std::is_integral_v<T>;

template <typename T, typename... Ts>
constexpr bool IsSame = (... && (std::is_same_v<T, Ts>));

template <typename T>
constexpr bool IsRValueReference = std::is_rvalue_reference_v<T &&>;

template <typename T>
constexpr bool IsFixedSizeArray = decltype(is_fixed_size_array(std::declval<T>()))::value;

} // namespace ltl

#define requires_f(x) ::ltl::requires_t<bool, (x)> = true
