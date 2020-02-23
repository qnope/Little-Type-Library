#pragma once

namespace ltl {
template <typename T> struct optional_type {
  constexpr optional_type(T) noexcept {}
  constexpr operator bool() const noexcept { return true; }

  constexpr auto operator*() const noexcept { return T{}; }
};

struct optional_empty {
  constexpr operator bool() const noexcept { return false; }
};
constexpr optional_empty nullopt_type;

} // namespace ltl
