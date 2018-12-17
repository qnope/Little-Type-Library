#pragma once

#include "../crtp.h"

namespace ltl {
template <typename T> struct EqualityComparable {
  constexpr friend bool operator==(const T &a, const T &b) {
    return a.get() == b.get();
  }

  constexpr friend bool operator!=(const T &a, const T &b) {
    return a.get() != b.get();
  }
};

#define OP(name, op)                                                           \
  template <typename T> struct name {                                          \
    constexpr friend bool operator op(const T &a, const T &b) {                \
      return a.get() > b.get();                                                \
    }                                                                          \
  };

OP(GreaterThan, >)
OP(LessThan, <)
OP(GreaterThanEqual, >=)
OP(LessThanEqual, <=)

#undef OP

} // namespace ltl
