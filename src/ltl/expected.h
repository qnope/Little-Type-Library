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

struct value_tag_t {};
struct error_tag_t {};

inline constexpr value_tag_t value_tag{};
inline constexpr error_tag_t error_tag{};

template <typename Result, typename Err>
/**
 * @brief The expected class
 *
 * expected<Result, Err> represents the monad error. It may only be a Result type or an Err type. It is basically an
 * equivalent to `std::variant<Result, Err>` with some syntactic sugar.
 * For people coming from Rust, it may reminds you the `Result` object.
 *
 * You may use it in different ways:
 *
 * The first way is the "procedural" one.
 * @code
 *  expected<std::vector<u8>, Error> readFile(std::string_view path);
 *
 *  auto fileReadExpected = readFile("file");
 *  if(fileReadExpected.is_result()) {
 *      const std::vector<u8> &result = fileReadExpected.result();
 *      use(result);
 *  } else {
 *      manageError(fileReadExpected.error());
 *  }
 * @endcode
 *
 * The second way, and the recommanded one, is the monadic function map and and_then.
 *
 * 1. map: it applies the function to the result and returns it, or returns the error if the expected contained an
 * error.
 * 2. and_then: The same as map, but the function returns an expected object.
 *
 * @code
 *  expected<std::vector<u8>, Error> readFile(std::string_view path);
 *  Processed simpleProcess(std::vector<u8> file);
 *  expected<Processed, Error> complicateProcess(std::vector<u8> file);
 *
 *  auto simpleResult = readFile("path.txt").map(simpleProcess);
 *  auto complicateResult = readFile("path.txt").and_then(complicateProcess);
 * @endcode
 *
 * The last way can be used when coroutines are available.
 *
 * @code
 *  expected<std::vector<u8>, Error> readFile(std::string_view path);
 *  Processed simpleProcess(std::vector<u8> file);
 *  expected<Processed, Error> complicateProcess(std::vector<u8> file);
 *  std::vector<u8> result = co_await readFile("path.txt");
 *  auto simpleResult = simpleProcess(result);
 *  auto complicateResult = co_await complicateProcess(result);
 * @endcode
 */
class [[nodiscard]] expected : public ltl::crtp::Comparable<expected<Result, Err>> {
    /// \cond
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
    constexpr expected(value_tag_t, T && t) : m_result{std::in_place_index<1>, FWD(t)} {}

    template <typename T>
    constexpr expected(error_tag_t, T && t) : m_result{std::in_place_index<2>, FWD(t)} {}

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

    template <typename... Fs>
    constexpr auto map(Fs && ... fs) //
        &->expected<ltl::remove_cvref_t<decltype(ltl::invoke(compose(FWD(fs)...), std::declval<value_type &>()))>,
                    error_type> {
        if (*this) {
            return ltl::invoke(compose(FWD(fs)...), this->result());
        }
        return this->error();
    }

    template <typename... Fs>
    constexpr auto map(Fs && ... fs) //
        const &->expected<
            ltl::remove_cvref_t<decltype(ltl::invoke(compose(FWD(fs)...), std::declval<const value_type &>()))>,
            error_type> {
        if (*this) {
            return ltl::invoke(compose(FWD(fs)...), this->result());
        }
        return this->error();
    }

    template <typename... Fs>
    constexpr auto map(Fs && ... fs) //
        &&->expected<ltl::remove_cvref_t<decltype(ltl::invoke(compose(FWD(fs)...), std::declval<value_type &&>()))>,
                     error_type> {
        if (*this) {
            return ltl::invoke(compose(FWD(fs)...), std::move(*this).result());
        }
        return std::move(*this).error();
    }

    template <typename... Fs>
    constexpr auto map(Fs && ... fs) //
        const &&->expected<
            ltl::remove_cvref_t<decltype(ltl::invoke(compose(FWD(fs)...), std::declval<const value_type &&>()))>,
            error_type> {
        if (*this) {
            return ltl::invoke(compose(FWD(fs)...), std::move(*this).result());
        }
        return std::move(*this).error();
    }

    template <typename... Fs>
    constexpr auto and_then(Fs && ... fs) //
        &->decltype(ltl::invoke(compose(FWD(fs)...), std::declval<value_type &>())) {
        if (*this) {
            return ltl::invoke(compose(FWD(fs)...), this->result());
        }
        return this->error();
    }

    template <typename... Fs>
    constexpr auto and_then(Fs && ... fs) //
        const &->decltype(ltl::invoke(compose(FWD(fs)...), std::declval<const value_type &>())) {
        if (*this) {
            return ltl::invoke(compose(FWD(fs)...), this->result());
        }
        return this->error();
    }

    template <typename... Fs>
    constexpr auto and_then(Fs && ... fs) //
        &&->decltype(ltl::invoke(compose(FWD(fs)...), std::declval<value_type &&>())) {
        if (*this) {
            return ltl::invoke(compose(FWD(fs)...), std::move(*this).result());
        }
        return std::move(*this).error();
    }

    template <typename... Fs>
    constexpr auto and_then(Fs && ... fs) //
        const &&->decltype(ltl::invoke(compose(FWD(fs)...), std::declval<const value_type &&>())) {
        if (*this) {
            return ltl::invoke(compose(FWD(fs)...), std::move(*this).result());
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
                std::remove_pointer_t<decltype(P::resultObject)>{error_tag, std::move(result).error()};
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

    /// \endcond
};

/// \cond
LTL_MAKE_IS_KIND(expected, is_expected, IsExpected, typename, ...);

template <typename T1, typename F, requires_f(IsExpected<T1>)>
constexpr decltype(auto) operator|(T1 &&a, MapType<F> b) {
    using value_type = decltype(ltl::fast_invoke(std::move(b.f), FWD(a).result()));
    using error_type = typename ltl::remove_cvref_t<T1>::error_type;
    if (a) {
        return expected<value_type, error_type>{value_tag, ltl::fast_invoke(std::move(b.f), FWD(a).result())};
    }
    return expected<value_type, error_type>{error_tag, FWD(a).error()};
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

    return return_type{error_tag, FWD(a).error()};
}

/// \endcond

///
/// \brief The Ok struct - A Simple structure to design a function that succeed when there is no other value to return
///
struct Ok {};

/// @}

} // namespace ltl
