#pragma once

#include <iterator>
#include <optional>

#include "ltl.h"
#include "traits.h"

namespace ltl {
using std::begin;
using std::end;
LTL_MAKE_IS_KIND(::std::optional, is_optional, is_optional_f, IsOptional, typename, );

template <typename T, typename = void>
constexpr bool IsIterable = false;

template <typename T>
constexpr bool IsIterable<T, std::void_t<decltype(begin(std::declval<T &>()), end(std::declval<T &>()))>> = true;

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

#define requires_f(x) ::std::enable_if_t<(x)> * = nullptr
