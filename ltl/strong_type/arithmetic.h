#pragma once
#include "../../lpl/cat.h"
#include "../crtp.h"

namespace ltl {
#define OP(name, op)                                                           \
  template <typename T> struct name : crtp<T, name> {                          \
    T &operator LPL_CAT(op, =)(const T &other) {                               \
      this->underlying().get() LPL_CAT(op, =) other.get();                     \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    constexpr friend T operator op(const T &a, const T &b) {                   \
      return T{a.get() op b.get()};                                            \
    }                                                                          \
  };

OP(Addable, +)
OP(Subtractable, -)
OP(Multipliable, *)
OP(Dividable, /)
OP(Moduloable, %)
#undef OP

} // namespace ltl
