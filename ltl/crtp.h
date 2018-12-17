#pragma once

namespace ltl {
template <typename T, template <typename> typename crtpType> struct crtp {
  T &underlying() { return static_cast<T &>(*this); }

  constexpr const T &underlying() const {
    return static_cast<const T &>(*this);
  }
};
} // namespace ltl
