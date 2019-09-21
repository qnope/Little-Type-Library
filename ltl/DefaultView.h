#pragma once

#include "range.h"

namespace ltl {

inline auto dereference() noexcept {
  return map([](auto &&x) noexcept->decltype(auto) { return *FWD(x); });
}

inline auto remove_null() noexcept {
  return filter([](auto &&x) noexcept { return static_cast<bool>(FWD(x)); });
}

template <int... Ns> auto get(number_t<Ns>...) {
  return map([](auto &&x) noexcept->decltype(auto) {
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
