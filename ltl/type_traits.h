#pragma once
#include "type_t.h"
#include <type_traits>

namespace ltl {
template <typename T>
[[nodiscard]] constexpr bool_t<std::is_default_constructible_v<T>>
is_default_constructible(type_t<T>) {
  return {};
}
} // namespace ltl
