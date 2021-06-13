/**
 * @file concept.h
 */
#pragma once

#include <iterator>
#include <optional>

#include "ltl.h"
#include "traits.h"

namespace ltl {
using std::begin;
using std::end;

LTL_MAKE_IS_KIND(::std::optional, is_optional, is_optional_f, IsOptional, typename, );

/// \cond
namespace detail {
template <typename T, typename = void>
constexpr bool IsIterableImpl = false;

template <typename T>
constexpr bool IsIterableImpl<T, std::void_t<decltype(begin(std::declval<T &>()), end(std::declval<T &>()))>> = true;
} // namespace detail

/// \endcond

template <typename T>
LTL_CONCEPT IsIterable = detail::IsIterableImpl<T>;

template <typename T>
LTL_CONCEPT IsConst = std::is_const_v<T>;

template <typename T>
LTL_CONCEPT IsFloatingPoint = std::is_floating_point_v<T>;

template <typename T>
LTL_CONCEPT IsIntegral = std::is_integral_v<T>;

template <typename T, typename... Ts>
LTL_CONCEPT IsSame = (... && (std::is_same_v<T, Ts>));

template <typename T>
LTL_CONCEPT IsRValueReference = std::is_rvalue_reference_v<T &&>;

template <typename T>
LTL_CONCEPT IsFixedSizeArray = decltype(is_fixed_size_array(std::declval<T>()))::value;

} // namespace ltl

#define requires_f(x) ::std::enable_if_t<(x)> * = nullptr
