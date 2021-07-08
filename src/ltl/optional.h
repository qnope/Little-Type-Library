/**
 * @file optional.h
 */
#pragma once

#include <optional>
#include "traits.h"
#include "crtp.h"
#include "functional.h"

#include "coroutine_helpers.h"

namespace ltl {

/**
 *\defgroup Utils Utilitary group
 *@{
 */

template <typename T>
/**
 * @brief The optional class
 *
 * It is the same as std::optional<T>
 * However, it provides a `map` function, and `and_then` function.
 * The map function is the _functor_ operator
 * The and_then is the _monad_ operator
 *
 * @code
 *  int square(int);
 *  ltl::optional<int> f(int);
 *  ltl::optional<int> x;
 *  auto y = x.map(square); // y = square(*x) or nullopt if x is empty
 *  auto z = x.and_then(f); // z = f(*x) or nullopt if x is empty
 * @endcode
 *
 * It provides also a better `value_or` function in the sense that it has a default parameter type.
 * It leads to the possibility to write
 *
 * @code
 *  ltl::optional<int> x;
 *  x.value_or({});
 * @endcode
 *
 * If coroutines are enabled, it also provides the co_await monadic operator
 *
 * @code
 *  ltl::optional<int> f();
 *
 *  ltl::optional<int> g() {
 *      co_return co_await f() * 2; // return f() * 2 or nullopt if f() returns a nullopt
 *  }
 * @endcode
 */
class optional : private std::optional<T>, public ltl::crtp::Comparable<optional<T>> {
    /// \cond
  public:
    using std::optional<T>::optional;
    using std::optional<T>::operator=;
    using std::optional<T>::operator->;
    using std::optional<T>::operator*;
    using std::optional<T>::operator bool;
    using std::optional<T>::has_value;
    using std::optional<T>::value;
    using std::optional<T>::swap;
    using std::optional<T>::reset;
    using std::optional<T>::emplace;

#if LTL_COROUTINE
    using promise_type = ltl::promise_type<optional<T>>;

    struct Awaiter {
        optional result;
        bool await_ready() noexcept { return result.has_value(); }
        void await_suspend(std::coroutine_handle<> handle) { handle.destroy(); }
        T await_resume() noexcept { return *std::move(result); }
    };

    Awaiter operator co_await() const & { return Awaiter{*this}; }
    Awaiter operator co_await() && { return Awaiter{std::move(*this)}; }

    optional(promise_type &promise) { promise.resultObject = this; }

    // Because we must not have a trivially copyable object, else it is allowed to passed by registers
    constexpr ~optional() {}
#endif

    template <typename U = T>
    constexpr T value_or(U x) const & {
        if (*this)
            return **this;
        return FWD(x);
    }

    template <typename U = T>
    constexpr T value_or(U &&x) && {
        if (*this)
            return std::move(**this);
        return FWD(x);
    }

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

    template <typename... Fs>
    constexpr auto map(Fs &&...fs) //
        & -> ltl::optional<ltl::remove_cvref_t<decltype(ltl::invoke(compose(FWD(fs)...), std::declval<T &>()))>> {
        if (*this) {
            return ltl::invoke(compose(FWD(fs)...), **this);
        }
        return std::nullopt;
    }

    template <typename... Fs>
    constexpr auto map(Fs &&...fs) //
        const & -> ltl::optional<
            ltl::remove_cvref_t<decltype(ltl::invoke(compose(FWD(fs)...), std::declval<const T &>()))>> {
        if (*this) {
            return ltl::invoke(compose(FWD(fs)...), **this);
        }
        return std::nullopt;
    }

    template <typename... Fs>
    constexpr auto map(Fs &&...fs) //
        && -> ltl::optional<ltl::remove_cvref_t<decltype(ltl::invoke(compose(FWD(fs)...), std::declval<T &&>()))>> {
        if (*this) {
            return ltl::invoke(compose(FWD(fs)...), std::move(**this));
        }
        return std::nullopt;
    }

    template <typename... Fs>
    constexpr auto map(Fs &&...fs) //
        const && -> ltl::optional<
            ltl::remove_cvref_t<decltype(ltl::invoke(compose(FWD(fs)...), std::declval<const T &&>()))>> {
        if (*this) {
            return ltl::invoke(compose(FWD(fs)...), std::move(**this));
        }
        return std::nullopt;
    }

    template <typename... Fs>
    constexpr auto and_then(Fs &&...fs) //
        & -> decltype(ltl::invoke(compose(FWD(fs)...), std::declval<T &>())) {
        if (*this) {
            return ltl::invoke(compose(FWD(fs)...), **this);
        }
        return std::nullopt;
    }

    template <typename... Fs>
    constexpr auto and_then(Fs &&...fs) //
        const & -> decltype(ltl::invoke(compose(FWD(fs)...), std::declval<const T &>())) {
        if (*this) {
            return ltl::invoke(compose(FWD(fs)...), **this);
        }
        return std::nullopt;
    }

    template <typename... Fs>
    constexpr auto and_then(Fs &&...fs) //
        && -> decltype(ltl::invoke(compose(FWD(fs)...), std::declval<T &&>())) {
        if (*this) {
            return ltl::invoke(compose(FWD(fs)...), std::move(**this));
        }
        return std::nullopt;
    }

    template <typename... Fs>
    constexpr auto and_then(Fs &&...fs) //
        const && -> decltype(ltl::invoke(compose(FWD(fs)...), std::declval<const T &&>())) {
        if (*this) {
            return ltl::invoke(compose(FWD(fs)...), std::move(**this));
        }
        return std::nullopt;
    }

    /// \endcond
};

/// \cond

template <class T>
optional(T) -> optional<T>;

/// \endcond

inline constexpr auto nullopt = ::std::nullopt;

template <typename T>
/**
 * @brief make_optional - build an optional
 * @param x
 */
constexpr auto make_optional(T x) {
    return optional{std::move(x)};
}

/// @}
} // namespace ltl
