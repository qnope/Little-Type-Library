/**
 * @file condition.h
 */
#pragma once

#include "Tuple.h"
#include "crtp.h"

namespace ltl {
/**
 * \defgroup Condition The Condition helper group
 * @{
 */

/// \cond
#define OP(op)                                                                                                         \
    template <typename T>                                                                                              \
    friend constexpr auto operator op(const AllOf &v, T &&t) noexcept {                                                \
        return v.m_values([&t](auto &&... xs) { return (true_v && ... && (FWD(xs) op t)); });                          \
    }                                                                                                                  \
    template <typename T>                                                                                              \
    friend constexpr auto operator op(T &&t, const AllOf &v) noexcept {                                                \
        return v.m_values([&t](auto &&... xs) { return (true_v && ... && (t op FWD(xs))); });                          \
    }
/// \endcond

template <typename... Ts>
/**
 * @brief The AllOf condition helper
 *
 * It is a simple helper to simplify condition
 * You can use it like this:
 * @code
 *  int a = 18;
 *  auto [b, c, d, e, f] = getSomeValues();
 *  if(a < ltl::AllOf{b, c, d, e, f})
 *      // Here a is less than b, c, d, e and f.
 * @endcode
 */
class AllOf : public crtp::Comparable<AllOf<Ts...>> {
    static_assert(sizeof...(Ts) > 0, "Never use AllOf without args");

  public:
    /// \cond
    constexpr AllOf() noexcept {}
    constexpr AllOf(Ts... ts) noexcept : m_values{std::move(ts)...} {}

    OP(<)
    OP(<=)
    OP(>)
    OP(>=)
    OP(==)
    OP(!=)

    constexpr operator bool() const noexcept { return (*this) == true; }
    /// \endcond
  private:
    ltl::tuple_t<Ts...> m_values;
};

/// \cond
template <typename... Ts>
AllOf(Ts...)->AllOf<Ts...>;

#undef OP

#define OP(op)                                                                                                         \
    template <typename T>                                                                                              \
    friend constexpr auto operator op(const AnyOf &v, T &&t) noexcept {                                                \
        return v.m_values([&t](auto &&... xs) { return (false_v || ... || (FWD(xs) op t)); });                         \
    }                                                                                                                  \
    template <typename T>                                                                                              \
    friend constexpr auto operator op(T &&t, const AnyOf &v) noexcept {                                                \
        return v.m_values([&t](auto &&... xs) { return (false_v || ... || (t op FWD(xs))); });                         \
    }
/// \endcond

template <typename... Ts>
/**
 * @brief The AnyOf condition helper
 *
 * It is a simple helper to simplify condition
 * You can use it like this:
 * @code
 *  int a = 18;
 *  auto [b, c, d, e, f] = getSomeValues();
 *  if(a == ltl::AnyOf{b, c, d, e, f})
 *      // Here a is equal to at least b, c, d, e or f.
 * @endcode
 */
class AnyOf : public crtp::Comparable<AnyOf<Ts...>> {
    static_assert(sizeof...(Ts) > 0, "Never use AnyOf without args");

  public:
    /// \cond
    constexpr AnyOf() noexcept {}
    constexpr AnyOf(Ts... ts) noexcept : m_values{std::move(ts)...} {}

    OP(<)
    OP(<=)
    OP(>)
    OP(>=)
    OP(==)
    OP(!=)

    constexpr operator bool() const noexcept { return (*this) == true; }
    /// \endcond
  private:
    ltl::tuple_t<Ts...> m_values;
};

/// \cond
template <typename... Ts>
AnyOf(Ts...)->AnyOf<Ts...>;

#undef OP

#define OP(op)                                                                                                         \
    template <typename T>                                                                                              \
    friend constexpr auto operator op(const NoneOf &v, T &&t) noexcept {                                               \
        return v.m_values([&t](auto &&... xs) { return !(false_v || ... || (FWD(xs) op t)); });                        \
    }                                                                                                                  \
    template <typename T>                                                                                              \
    friend constexpr auto operator op(T &&t, const NoneOf &v) noexcept {                                               \
        return v.m_values([&t](auto &&... xs) { return !(false_v || ... || (t op FWD(xs))); });                        \
    }
/// \endcond

template <typename... Ts>
/**
 * @brief The NoneOf condition helper
 *
 * It is a simple helper to simplify condition
 * You can use it like this:
 * @code
 *  int a = 18;
 *  auto [b, c, d, e, f] = getSomeValues();
 *  if(a == ltl::NoneOf{b, c, d, e, f})
 *      // Here a is equal to none of b, c, d, e or f.
 * @endcode
 */
class NoneOf : public crtp::Comparable<AllOf<Ts...>> {
    static_assert(sizeof...(Ts) > 0, "Never use NoneOf without args");

  public:
    /// \cond
    constexpr NoneOf() noexcept {}
    constexpr NoneOf(Ts... ts) noexcept : m_values{std::move(ts)...} {}

    OP(<)
    OP(<=)
    OP(>)
    OP(>=)
    OP(==)
    OP(!=)
    constexpr operator bool() const noexcept { return (*this) == true; }
    /// \endcond

  private:
    ltl::tuple_t<Ts...> m_values;
};

/// \cond
template <typename... Ts>
NoneOf(Ts...)->NoneOf<Ts...>;

#undef OP

template <typename... Ts>
constexpr auto AnyOfT = AnyOf<type_t<Ts>...>{};

template <typename... Ts>
constexpr auto AllOfT = AllOf<type_t<Ts>...>{};

template <typename... Ts>
constexpr auto NoneOfT = NoneOf<type_t<Ts>...>{};
/// \endcond
/// @}

} // namespace ltl
