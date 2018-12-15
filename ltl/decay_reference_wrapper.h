#pragma once
#include <type_traits>
#include <utility>

namespace ltl {
template <typename T> struct decay_reference_wrapper {
  using type = std::decay_t<T>;
};

template <typename T>
struct decay_reference_wrapper<std::reference_wrapper<T>> {
  using type = T &;
};

template <typename T>
using decay_reference_wrapper_t = typename decay_reference_wrapper<T>::type;
} // namespace ltl
