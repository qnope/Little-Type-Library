#pragma once

#include <optional>
#include "invoke.h"
#include "crtp.h"

namespace ltl {

template <typename T>
class optional : private std::optional<T>, public ltl::crtp::Comparable<optional<T>> {
  public:
    using std::optional<T>::optional;
    using std::optional<T>::operator=;
    using std::optional<T>::operator->;
    using std::optional<T>::operator*;
    using std::optional<T>::operator bool;
    using std::optional<T>::has_value;
    using std::optional<T>::value;
    using std::optional<T>::value_or;
    using std::optional<T>::swap;
    using std::optional<T>::reset;
    using std::optional<T>::emplace;

    constexpr friend bool operator==(const optional &a, const optional &b) noexcept {
        return static_cast<const std::optional<T> &>(a) == static_cast<const std::optional<T> &>(b);
    }

    template <typename _T>
    constexpr friend bool operator==(const _T &a, const optional &b) noexcept {
        return static_cast<const _T &>(a) == static_cast<const std::optional<T> &>(b);
    }

    template <typename _T>
    constexpr friend bool operator==(const optional &a, const _T &b) noexcept {
        return static_cast<const std::optional<T> &>(a) == static_cast<const _T &>(b);
    }

    constexpr friend bool operator<(const optional &a, const optional &b) noexcept {
        return static_cast<const std::optional<T> &>(a) < static_cast<const std::optional<T> &>(b);
    }

    template <typename _T>
    constexpr friend bool operator<(const _T &a, const optional &b) noexcept {
        return static_cast<const _T &>(a) < static_cast<const std::optional<T> &>(b);
    }

    template <typename _T>
    constexpr friend bool operator<(const optional &a, const _T &b) noexcept {
        return static_cast<const std::optional<T> &>(a) < static_cast<const _T &>(b);
    }

    template <typename F>
    constexpr auto map(F &&f) //
        & -> ltl::optional<ltl::remove_cvref_t<decltype(ltl::invoke(FWD(f), std::declval<T &>()))>> {
        if (*this) {
            return ltl::invoke(FWD(f), **this);
        }
        return std::nullopt;
    }

    template <typename F>
    constexpr auto map(F &&f) //
        const & -> ltl::optional<ltl::remove_cvref_t<decltype(ltl::invoke(FWD(f), std::declval<const T &>()))>> {
        if (*this) {
            return ltl::invoke(FWD(f), **this);
        }
        return std::nullopt;
    }

    template <typename F>
    constexpr auto map(F &&f) //
        && -> ltl::optional<ltl::remove_cvref_t<decltype(ltl::invoke(FWD(f), std::declval<T &&>()))>> {
        if (*this) {
            return ltl::invoke(FWD(f), std::move(**this));
        }
        return std::nullopt;
    }

    template <typename F>
    constexpr auto map(F &&f) //
        const && -> ltl::optional<ltl::remove_cvref_t<decltype(ltl::invoke(FWD(f), std::declval<const T &&>()))>> {
        if (*this) {
            return ltl::invoke(FWD(f), std::move(**this));
        }
        return std::nullopt;
    }

    template <typename F>
    constexpr auto and_then(F &&f) //
        & -> decltype(ltl::invoke(FWD(f), std::declval<T &>())) {
        if (*this) {
            return ltl::invoke(FWD(f), **this);
        }
        return std::nullopt;
    }

    template <typename F>
    constexpr auto and_then(F &&f) //
        const & -> decltype(ltl::invoke(FWD(f), std::declval<const T &>())) {
        if (*this) {
            return ltl::invoke(FWD(f), **this);
        }
        return std::nullopt;
    }

    template <typename F>
    constexpr auto and_then(F &&f) //
        && -> decltype(ltl::invoke(FWD(f), std::declval<T &&>())) {
        if (*this) {
            return ltl::invoke(FWD(f), std::move(**this));
        }
        return std::nullopt;
    }

    template <typename F>
    constexpr auto and_then(F &&f) //
        const && -> decltype(ltl::invoke(FWD(f), std::declval<const T &&>())) {
        if (*this) {
            return ltl::invoke(FWD(f), std::move(**this));
        }
        return std::nullopt;
    }
};

template <class T>
optional(T)->optional<T>;

inline constexpr auto nullopt = ::std::nullopt;

template <typename T>
constexpr auto make_optional(T x) {
    return optional{std::move(x)};
}
} // namespace ltl
