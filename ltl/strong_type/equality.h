#pragma once
#include "../crtp.h"
#include "../type_t.h"

namespace ltl {

#define OP(name, op)                                                           \
  template <typename T> struct name {                                          \
    [[nodiscard]] constexpr friend bool operator op(const T &a, const T &b) {  \
      return a.get() op b.get();                                               \
    }                                                                          \
                                                                               \
    template <typename T2>                                                     \
    [[nodiscard]] constexpr friend bool operator op(const T &a, const T2 &b) { \
      typed_static_assert(a.isSameKind(b));                                    \
      return a.get() op static_cast<T>(b).get();                               \
    }                                                                          \
  };

OP(GreaterThan, >)
OP(LessThan, <)
OP(GreaterThanEqual, >=)
OP(LessThanEqual, <=)

namespace detail {
OP(LTLSTEquality, ==)
OP(LTLSTInequality, !=)
} // namespace detail

template <typename T>
struct EqualityComparable : detail::LTLSTEquality<T>,
                            detail::LTLSTInequality<T> {};

template <typename T>
struct AllComparable : EqualityComparable<T>,
                       GreaterThan<T>,
                       LessThan<T>,
                       GreaterThanEqual<T>,
                       LessThanEqual<T> {};

#undef OP

} // namespace ltl
