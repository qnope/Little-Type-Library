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

#define OP(name, op)                                                                     \
  template <typename T> struct name {                                                    \
    ENABLE_CRTP(T)                                                                       \
    template <typename _T> friend T operator op(T v, const _T &t) {                      \
      v LPL_CAT(op, =) t;                                                                \
      return v;                                                                          \
    }                                                                                    \
    template <typename _T> friend T operator op(const _T &t, T v) {                      \
      v LPL_CAT(op, =) t;                                                                \
      return v;                                                                          \
    }                                                                                    \
  };

OP(Additionnable, +)
OP(Substractable, -)
#undef OP

} // namespace ltl