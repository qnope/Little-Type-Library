#pragma once

#include <type_traits>
#include <utility>

#include "invoke.h"

#include "lpl/lpl.h"

#define decltype_t(t) ltl::extract_type<decltype(t)>

#define type_from(x) ltl::type_v<decltype(x)>
#define decay_from(x) ltl::type_v<std::decay_t<decltype(x)>>

#define typed_static_assert(f) static_assert(decltype(f){})
#define typed_static_assert_msg(f, msg) static_assert(decltype(f){}, msg)
#define if_constexpr(c) if constexpr (decltype(c){})
#define else_if_constexpr(c) else if constexpr (decltype(c){})

#define compile_time_error(msg, T) static_assert(ltl::always_false<T>, msg);

#define FWD(x) static_cast<decltype(x) &&>(x)

#define lift(f) [](auto &&... xs) -> decltype(f(FWD(xs)...)) { return f(FWD(xs)...); }

namespace ltl {
template <typename>
constexpr bool always_false = false;

/////////////////////// decay_reference_wrapper
template <typename T>
struct decay_reference_wrapper {
    using type = std::decay_t<T>;
};

template <typename T>
struct decay_reference_wrapper<std::reference_wrapper<T>> {
    using type = T &;
};

template <typename T>
using decay_reference_wrapper_t = typename decay_reference_wrapper<T>::type;

template <typename T>
using remove_rvalue_reference_t = std::conditional_t<std::is_lvalue_reference_v<T>, T, std::decay_t<T>>;

///////////////////// overloader
template <typename... Fs>
struct overloader : Fs... {
    constexpr overloader(Fs... fs) : Fs{std::move(fs)}... {}
    using Fs::operator()...;
};

///////////////////// bool
template <bool v>
struct bool_t {
    static constexpr bool value = v;
    constexpr operator bool() const { return v; }
};

using false_t = bool_t<false>;
using true_t = bool_t<true>;
} // namespace ltl

constexpr ltl::false_t false_v;
constexpr ltl::true_t true_v;

namespace ltl {
template <bool v>
constexpr bool_t<v> bool_v{};

#define OP(op)                                                                                                         \
    template <bool v1, bool v2>                                                                                        \
    [[nodiscard]] constexpr bool_t<v1 op v2> operator op(bool_t<v1>, bool_t<v2>) {                                     \
        return {};                                                                                                     \
    }

OP(==)
OP(!=)
OP(&&)
OP(||)

#undef OP

template <bool v>
[[nodiscard]] constexpr bool_t<!v> operator!(bool_t<v>) {
    return {};
}

///////////////////////// type
template <typename T>
struct type_t {
    using type = T;
};

template <typename T>
constexpr type_t<T> type_v{};

template <typename T1, typename T2>
[[nodiscard]] constexpr false_t operator==(type_t<T1>, type_t<T2>) {
    return {};
}

template <typename T>
[[nodiscard]] constexpr true_t operator==(type_t<T>, type_t<T>) {
    return {};
}

template <typename T1, typename T2>
[[nodiscard]] constexpr true_t operator!=(type_t<T1>, type_t<T2>) {
    return {};
}

template <typename T>
[[nodiscard]] constexpr false_t operator!=(type_t<T>, type_t<T>) {
    return {};
}

template <typename T>
using extract_type = typename std::decay_t<T>::type;

////////////////////// number
template <int N>
struct number_t {
    constexpr static int value = N;
};

namespace detail {
template <char... _digits>
[[nodiscard]] constexpr int digits_to_int() {
    constexpr char digits[] = {_digits...};
    int result = 0;
    for (int digit : digits) {
        result *= 10;
        result += digit - '0';
    }
    return result;
}
} // namespace detail

#define OP(op)                                                                                                         \
    template <int N1, int N2>                                                                                          \
    [[nodiscard]] constexpr number_t<(N1 op N2)> operator op(number_t<N1>, number_t<N2>) {                             \
        return {};                                                                                                     \
    }
OP(+)
OP(-)
OP(*)
OP(/)
OP(%)
OP(&)
OP(|)
OP(^)
OP(<<)
OP(>>)
#undef OP

#define OP(op)                                                                                                         \
    template <int N1, int N2>                                                                                          \
    [[nodiscard]] constexpr bool_t<(N1 op N2)> operator op(number_t<N1>, number_t<N2>) {                               \
        return {};                                                                                                     \
    }
OP(==)
OP(!=)
OP(<)
OP(<=)
OP(>)
OP(>=)
#undef OP

#define OP(op)                                                                                                         \
    template <int N>                                                                                                   \
    [[nodiscard]] constexpr number_t<(op N)> operator op(number_t<N>) {                                                \
        return {};                                                                                                     \
    }
OP(~)
OP(+)
OP(-)
#undef OP
} // namespace ltl

template <int N>
constexpr ltl::number_t<N> number_v{};
template <char... digits>
[[nodiscard]] constexpr auto operator""_n() {
    return number_v<ltl::detail::digits_to_int<digits...>()>;
}

namespace ltl {
[[nodiscard]] constexpr auto bool_to_number(false_t) { return 0_n; }
[[nodiscard]] constexpr auto bool_to_number(true_t) { return 1_n; }

template <typename T>
[[nodiscard]] constexpr auto max_type(T a) {
    return a;
}

template <typename T1, typename T2, typename... Ts>
[[nodiscard]] constexpr auto max_type(T1 a, T2 b, Ts... ts) {
    if_constexpr(a > b) return ::ltl::max_type(a, ts...);
    else return ::ltl::max_type(b, ts...);
}

template <typename T>
[[nodiscard]] constexpr auto min_type(T a) {
    return a;
}
template <typename T1, typename T2, typename... Ts>
[[nodiscard]] constexpr auto min_type(T1 a, T2 b, Ts... ts) {
    if_constexpr(a < b) return ::ltl::min_type(a, ts...);
    else return ::ltl::min_type(b, ts...);
}

template <typename... Ts>
struct CompileError;

false_t is_bool_t(...) noexcept;
false_t is_type_t(...) noexcept;
false_t is_number_t(...) noexcept;

template <bool B>
true_t is_bool_t(bool_t<B>);
template <typename T>
true_t is_type_t(type_t<T>);
template <int N>
true_t is_number_t(number_t<N>);

} // namespace ltl
