/**
 * @file expected.h
 */
#pragma once

#include <cassert>
#include <utility>
#include <variant>

#include "Range/Map.h"
#include "coroutine_helpers.h"

namespace ltl {

/**
 *\defgroup Utils Utilitary group
 *@{
 */

struct value_tag {};
struct error_tag {};

template <typename Result, typename Err>
class [[nodiscard]] expected : public ltl::crtp::Comparable<expected<Result, Err>> {
  public:
    using value_type = Result;
    using error_type = Err;

    constexpr friend bool operator==(const expected &a, const expected &b) noexcept { return a.m_result == b.m_result; }

    static_assert(!std::is_reference_v<value_type>, "value_type must not be a reference");
    static_assert(!std::is_reference_v<error_type>, "error_type must not be a reference");

    template <typename T>
    constexpr expected(T && t) : m_result{FWD(t)} {}

    constexpr expected(expected &&) = default;
    constexpr expected(const expected &) = default;
    constexpr expected &operator=(expected &&) = default;
    constexpr expected &operator=(const expected &) = default;

    template <typename T>
    constexpr expected(value_tag, T && t) : m_result{std::in_place_index<1>, FWD(t)} {}

    template <typename T>
    constexpr expected(error_tag, T && t) : m_result{std::in_place_index<2>, FWD(t)} {}

    template <typename T, typename E>
    constexpr expected(expected<T, E> t) {
        if (t)
            m_result = value_type(std::move(t).result());
        else
            m_result = error_type(std::move(t).error());
    }

    template <typename R, typename E>
    constexpr expected &operator=(expected<R, E> t) {
        if (t)
            m_result = value_type(std::move(t).result());
        else
            m_result = error_type(std::move(t).error());
        return *this;
    }

    template <typename T>
    constexpr expected &operator=(T) = delete;

    constexpr operator bool() const noexcept { return m_result.index() == 1; }

    constexpr value_type &result() & noexcept {
        assert(m_result.index() == 1);
        return std::get<1>(m_result);
    }

    constexpr const value_type &result() const &noexcept {
        assert(m_result.index() == 1);
        return std::get<1>(m_result);
    }

    constexpr value_type result() && noexcept {
        assert(m_result.index() == 1);
        return std::get<1>(std::move(m_result));
    }

    constexpr const value_type result() const &&noexcept {
        assert(m_result.index() == 1);
        return std::get<1>(std::move(m_result));
    }

    constexpr error_type error() && noexcept {
        assert(m_result.index() == 2);
        return std::get<2>(std::move(m_result));
    }

    constexpr error_type error() const &&noexcept {
        assert(m_result.index() == 2);
        return std::get<2>(std::move(m_result));
    }

    constexpr const error_type &error() const &noexcept {
        assert(m_result.index() == 2);
        return std::get<2>(m_result);
    }

    constexpr error_type &error() & noexcept {
        assert(m_result.index() == 2);
        return std::get<2>(m_result);
    }

    template <typename F>
    constexpr auto map(F && f) //
        &->expected<ltl::remove_cvref_t<decltype(ltl::invoke(FWD(f), std::declval<value_type &>()))>, error_type> {
        if (*this) {
            return ltl::invoke(FWD(f), this->result());
        }
        return this->error();
    }

    template <typename F>
    constexpr auto map(F && f) //
        const &->expected<ltl::remove_cvref_t<decltype(ltl::invoke(FWD(f), std::declval<const value_type &>()))>,
                          error_type> {
        if (*this) {
            return ltl::invoke(FWD(f), this->result());
        }
        return this->error();
    }

    template <typename F>
    constexpr auto map(F && f) //
        &&->expected<ltl::remove_cvref_t<decltype(ltl::invoke(FWD(f), std::declval<value_type &&>()))>, error_type> {
        if (*this) {
            return ltl::invoke(FWD(f), std::move(*this).result());
        }
        return std::move(*this).error();
    }

    template <typename F>
    constexpr auto map(F && f) //
        const &&->expected<ltl::remove_cvref_t<decltype(ltl::invoke(FWD(f), std::declval<const value_type &&>()))>,
                           error_type> {
        if (*this) {
            return ltl::invoke(FWD(f), std::move(*this).result());
        }
        return std::move(*this).error();
    }

    template <typename F>
    constexpr auto and_then(F && f) //
        &->decltype(ltl::invoke(FWD(f), std::declval<value_type &>())) {
        if (*this) {
            return ltl::invoke(FWD(f), this->result());
        }
        return this->error();
    }

    template <typename F>
    constexpr auto and_then(F && f) //
        const &->decltype(ltl::invoke(FWD(f), std::declval<const value_type &>())) {
        if (*this) {
            return ltl::invoke(FWD(f), this->result());
        }
        return this->error();
    }

    template <typename F>
    constexpr auto and_then(F && f) //
        &&->decltype(ltl::invoke(FWD(f), std::declval<value_type &&>())) {
        if (*this) {
            return ltl::invoke(FWD(f), std::move(*this).result());
        }
        return std::move(*this).error();
    }

    template <typename F>
    constexpr auto and_then(F && f) //
        const &&->decltype(ltl::invoke(FWD(f), std::declval<const value_type &&>())) {
        if (*this) {
            return ltl::invoke(FWD(f), std::move(*this).result());
        }
        return std::move(*this).error();
    }

    constexpr bool is_error() const noexcept { return m_result.index() == 2; }
    constexpr bool is_result() const noexcept { return m_result.index() == 1; }

#if LTL_COROUTINE
    using promise_type = ltl::promise_type<expected<Result, Err>>;

    struct Awaiter {
        expected result{};
        bool await_ready() noexcept { return result.is_result(); }
        template <typename P>
        void await_suspend(std::coroutine_handle<P> handle) {
            *handle.promise().resultObject =
                std::remove_pointer_t<decltype(P::resultObject)>{error_tag{}, std::move(result).error()};
            handle.destroy();
        }
        Result await_resume() noexcept { return std::move(result).result(); }
    };

    Awaiter operator co_await() const & { return {*this}; }
    Awaiter operator co_await() && { return {std::move(*this)}; }

    expected(promise_type & promise) { promise.resultObject = this; }

    // Because we must not have a trivially copyable object, else it is allowed to passed by registers
    constexpr ~expected() {}
#endif

  private:
    std::variant<std::monostate, value_type, error_type> m_result;
};

LTL_MAKE_IS_KIND(expected, is_expected, is_expected_f, IsExpected, typename, ...);

template <typename T1, typename F, requires_f(IsExpected<T1>)>
constexpr decltype(auto) operator|(T1 &&a, MapType<F> b) {
    using value_type = decltype(ltl::fast_invoke(std::move(b.f), FWD(a).result()));
    using error_type = typename ltl::remove_cvref_t<T1>::error_type;
    if (a) {
        return expected<value_type, error_type>{value_tag{}, ltl::fast_invoke(std::move(b.f), FWD(a).result())};
    }
    return expected<value_type, error_type>{error_tag{}, FWD(a).error()};
}

template <typename T1, typename F, requires_f(IsExpected<T1>)>
constexpr decltype(auto) operator>>(T1 &&a, MapType<F> b) {
    static_assert(IsExpected<decltype(ltl::fast_invoke(std::move(b.f), FWD(a).result()))>,
                  "With >> notation, function must return an expected");
    using old_error_type = typename ltl::remove_cvref_t<T1>::error_type;
    using return_type = ltl::remove_cvref_t<decltype(ltl::fast_invoke(std::move(b.f), FWD(a).result()))>;
    using new_error_type = typename return_type::error_type;

    static_assert(std::is_convertible_v<old_error_type, new_error_type>,
                  "Old error type must be convertible to new_error_type");

    if (a)
        return ltl::fast_invoke(std::move(b.f), FWD(a).result());

    return return_type{error_tag{}, FWD(a).error()};
}

struct Ok {};

/// @}

} // namespace ltl
