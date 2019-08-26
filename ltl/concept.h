#pragma once

#include "ltl.h"

namespace ltl {
template <typename T, bool b> using requires_t = std::enable_if_t<b, T>;
template <bool b> using requires_void = requires_t<void, b>;

template <typename T>
constexpr auto IsIterable = decltype(is_iterable(std::declval<T>())){};
template <typename T>
constexpr auto IsConst = decltype(is_const(std::declval<T>())){};

template <typename T>
constexpr auto IsFloatingPoint =
    decltype(is_floating_point(std::declval<T>())){};

template <typename T>
constexpr auto IsIntegral = decltype(is_integral(std::declval<T>())){};

template <typename T, typename... Ts>
constexpr auto IsSame =
    decltype((true_v && ... && (ltl::type_v<T> == ltl::type_v<Ts>))){};

} // namespace ltl

#define requires_f(x) ::ltl::requires_t<bool, (x)> = true
