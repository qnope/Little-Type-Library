#pragma once

#include "Tuple.h"
#include "crtp.h"

namespace ltl {
#define OP(op)                                                                                                         \
    template <typename T>                                                                                              \
    friend constexpr auto operator op(const AllOf &v, T &&t) noexcept {                                                \
        return v.m_values([&t](auto &&...xs) { return (true_v && ... && (FWD(xs) op t)); });                           \
    }                                                                                                                  \
    template <typename T>                                                                                              \
    friend constexpr auto operator op(T &&t, const AllOf &v) noexcept {                                                \
        return v.m_values([&t](auto &&...xs) { return (true_v && ... && (t op FWD(xs))); });                           \
    }

template <typename... Ts>
class AllOf : public crtp::Comparable<AllOf<Ts...>> {
    static_assert(sizeof...(Ts) > 0, "Never use AllOf without args");

  public:
    constexpr AllOf(Ts... ts) noexcept : m_values{std::move(ts)...} {}

    OP(<)
    OP(<=)
    OP(>)
    OP(>=)
    OP(==)
    OP(!=)

    constexpr operator bool() const noexcept { return (*this) == true; }

  private:
    ltl::tuple_t<Ts...> m_values;
};

template <typename... Ts>
AllOf(Ts...) -> AllOf<Ts...>;

#undef OP

#define OP(op)                                                                                                         \
    template <typename T>                                                                                              \
    friend constexpr auto operator op(const AnyOf &v, T &&t) noexcept {                                                \
        return v.m_values([&t](auto &&...xs) { return (false_v || ... || (FWD(xs) op t)); });                          \
    }                                                                                                                  \
    template <typename T>                                                                                              \
    friend constexpr auto operator op(T &&t, const AnyOf &v) noexcept {                                                \
        return v.m_values([&t](auto &&...xs) { return (false_v || ... || (t op FWD(xs))); });                          \
    }

template <typename... Ts>
class AnyOf : public crtp::Comparable<AnyOf<Ts...>> {
    static_assert(sizeof...(Ts) > 0, "Never use AnyOf without args");

  public:
    constexpr AnyOf(Ts... ts) noexcept : m_values{std::move(ts)...} {}

    OP(<)
    OP(<=)
    OP(>)
    OP(>=)
    OP(==)
    OP(!=)

    constexpr operator bool() const noexcept { return (*this) == true; }

  private:
    ltl::tuple_t<Ts...> m_values;
};

template <typename... Ts>
AnyOf(Ts...) -> AnyOf<Ts...>;

#undef OP

#define OP(op)                                                                                                         \
    template <typename T>                                                                                              \
    friend constexpr auto operator op(const NoneOf &v, T &&t) noexcept {                                               \
        return v.m_values([&t](auto &&...xs) { return !(false_v || ... || (FWD(xs) op t)); });                         \
    }                                                                                                                  \
    template <typename T>                                                                                              \
    friend constexpr auto operator op(T &&t, const NoneOf &v) noexcept {                                               \
        return v.m_values([&t](auto &&...xs) { return !(false_v || ... || (t op FWD(xs))); });                         \
    }

template <typename... Ts>
class NoneOf : public crtp::Comparable<AllOf<Ts...>> {
    static_assert(sizeof...(Ts) > 0, "Never use NoneOf without args");

  public:
    constexpr NoneOf(Ts... ts) noexcept : m_values{std::move(ts)...} {}

    OP(<)
    OP(<=)
    OP(>)
    OP(>=)
    OP(==)
    OP(!=)
    constexpr operator bool() const noexcept { return (*this) == true; }

  private:
    ltl::tuple_t<Ts...> m_values;
};

template <typename... Ts>
NoneOf(Ts...) -> NoneOf<Ts...>;

#undef OP

} // namespace ltl
