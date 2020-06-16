#pragma once

#include "../Tuple.h"
#include "Filter.h"
#include "Map.h"

namespace ltl {
namespace detail {
template <typename T>
struct remove_rvalue_reference {
  using type = std::decay_t<T>;
};

template <typename T>
struct remove_rvalue_reference<T &> {
  using type = T &;
};

template <typename T>
using remove_rvalue_reference_t = typename remove_rvalue_reference<T>::type;
} // namespace detail

inline auto dereference() noexcept {
  return map([](auto &&x) noexcept -> detail::remove_rvalue_reference_t<decltype(*FWD(x))> { return *FWD(x); });
}

inline auto remove_null() noexcept {
  return filter([](auto &&x) noexcept { return static_cast<bool>(FWD(x)); });
}

template <int... Ns>
auto get(number_t<Ns>...) {
  return map([](auto &&x) noexcept -> decltype(auto) {
    static_assert(sizeof...(Ns) > 0, "You must provide at least one value to get");

    if constexpr (sizeof...(Ns) == 1) {
      return (::std::get<static_cast<std::size_t>(Ns)>(FWD(x)), ...);
    }

    else {
      return ltl::tuple_t<decltype(::std::get<static_cast<std::size_t>(Ns)>(FWD(x)))...>{
          ::std::get<static_cast<std::size_t>(Ns)>(FWD(x))...};
    }
  });
}

inline auto keys() { return get(0_n); }
inline auto values() { return get(1_n); }

} // namespace ltl
