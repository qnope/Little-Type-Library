#pragma once
#include "type_t.h"
#include <type_traits>

namespace ltl {
template <typename T>
constexpr std::conditional_t<std::is_default_constructible_v<T>, true_t,
                             false_t>
is_default_constructible(type_t<T>) {
  return {};
}
} // namespace ltl
