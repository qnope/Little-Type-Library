#pragma once

#include "template.h"

namespace ltl {
template <typename N1, typename N2>
[[nodiscard]] constexpr auto build_index_sequence(N1 n1, N2 n2) {
  typed_static_assert(is_number_t(n1));
  typed_static_assert(is_number_t(n2));
  typed_static_assert(n1 < n2);
  return tuple_t<>::build_index_sequence(n1, n2);
}

template <typename N>[[nodiscard]] constexpr auto build_index_sequence(N n) {
  return build_index_sequence(0_n, n);
}
} // namespace ltl
