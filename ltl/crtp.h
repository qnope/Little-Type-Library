#pragma once

#include "../lpl/lpl.h"

#define ENABLE_CRTP(DerivedType)                                                         \
  constexpr DerivedType &underlying() noexcept {                                         \
    return static_cast<DerivedType &>(*this);                                            \
  }                                                                                      \
  constexpr const DerivedType &underlying() const noexcept {                             \
    return static_cast<const DerivedType &>(*this);                                      \
  }

namespace ltl {
template <typename Derived> class Comparable {
  ENABLE_CRTP(Derived)

public:
  template <typename Other> constexpr bool operator!=(const Other &o) const noexcept {
    return !(underlying() == o);
  }

  template <typename Other> constexpr bool operator<=(const Other &o) const noexcept {
    return (underlying() < o) || (underlying() == o);
  }

  template <typename Other> constexpr bool operator>(const Other &o) const noexcept {
    return !(*this <= o);
  }

  template <typename Other> constexpr bool operator>=(const Other &o) const noexcept {
    return (*this > o) || (underlying() == o);
  }
};

#define OP(name, op)                                                                     \
  template <typename T> struct name {                                                    \
    ENABLE_CRTP(T)                                                                       \
    constexpr T operator op(int) {                                                       \
      auto tmp = underlying();                                                           \
      op underlying();                                                                   \
      return tmp;                                                                        \
    }                                                                                    \
  };

OP(PostIncrementable, ++)
OP(PostDecrementable, --)

#undef OP

} // namespace ltl
