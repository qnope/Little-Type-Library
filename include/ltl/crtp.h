#pragma once

#include "lpl/lpl.h"

#define ENABLE_CRTP(DerivedType)                                                                                       \
    constexpr DerivedType &underlying() noexcept { return static_cast<DerivedType &>(*this); }                         \
    constexpr const DerivedType &underlying() const noexcept { return static_cast<const DerivedType &>(*this); }

namespace ltl {
template <typename Derived>
class Comparable {
  public:
    friend constexpr auto operator!=(const Derived &a, const Derived &b) noexcept { return !(a == b); }

    friend constexpr auto operator<=(const Derived &a, const Derived &b) noexcept { return !(b < a); }

    friend constexpr auto operator>(const Derived &a, const Derived &b) noexcept { return b < a; }

    friend constexpr auto operator>=(const Derived &a, const Derived &b) noexcept { return !(a < b); }
};

#define OP(name, op)                                                                                                   \
    template <typename T>                                                                                              \
    struct name {                                                                                                      \
        ENABLE_CRTP(T)                                                                                                 \
        constexpr friend T operator op(T &v, int) {                                                                    \
            auto tmp = v;                                                                                              \
            op v;                                                                                                      \
            return tmp;                                                                                                \
        }                                                                                                              \
    };

OP(PostIncrementable, ++)
OP(PostDecrementable, --)
#undef OP

#define OP(name, op)                                                                                                   \
    template <typename T>                                                                                              \
    struct name {                                                                                                      \
        ENABLE_CRTP(T)                                                                                                 \
        template <typename _T>                                                                                         \
        friend T operator op(T v, const _T &t) {                                                                       \
            v LPL_CAT(op, =) t;                                                                                        \
            return v;                                                                                                  \
        }                                                                                                              \
        template <typename _T>                                                                                         \
        friend T operator op(const _T &t, T v) {                                                                       \
            v LPL_CAT(op, =) t;                                                                                        \
            return v;                                                                                                  \
        }                                                                                                              \
    };

OP(Additionnable, +)
OP(Substractable, -)
#undef OP

} // namespace ltl
