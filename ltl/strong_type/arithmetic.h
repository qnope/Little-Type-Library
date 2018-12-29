#pragma once
#include "../../lpl/cat.h"
#include "../crtp.h"
#include "../type_t.h"

namespace ltl {
#define OP(name, op)                                                           \
  template <typename T> struct name : crtp<T, name> {                          \
    T &operator LPL_CAT(op, =)(const T &other) {                               \
      this->underlying().get() LPL_CAT(op, =) other.get();                     \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    [[nodiscard]] constexpr friend T operator op(const T &a, const T &b) {     \
      return T{a.get() op b.get()};                                            \
    }                                                                          \
                                                                               \
    template <typename T2> T &operator LPL_CAT(op, =)(const T2 &other) {       \
      typed_static_assert(this->underlying().isSameKind(other));               \
      this->underlying().get() LPL_CAT(op, =) static_cast<T>(other).get();     \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    template <typename T2>                                                     \
    [[nodiscard]] constexpr friend T operator op(const T &a, const T2 &b) {    \
      typed_static_assert(a.isSameKind(b));                                    \
      return T{a.get() op static_cast<T>(b).get()};                            \
    }                                                                          \
  };

OP(Addable, +)
OP(Subtractable, -)
OP(Multipliable, *)
OP(Dividable, /)
OP(Moduloable, %)

#undef OP

#define OP(name, op)                                                           \
  template <typename T> struct name : crtp<T, name> {                          \
    constexpr T &operator op() {                                               \
      op this->underlying().get();                                             \
      return this->underlying();                                               \
    }                                                                          \
                                                                               \
    constexpr T operator op(int) {                                             \
      this->underlying().get() op;                                             \
      return *this->underlying();                                              \
    }                                                                          \
  };

OP(Incrementable, ++)
OP(Decrementable, --)

#undef OP
} // namespace ltl
