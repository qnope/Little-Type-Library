#pragma once

#include "ltl.h"
#include "traits.h"
#include <cassert>
#include <utility>
#include <variant>

namespace ltl {

struct value_tag {};
struct error_tag {};

template <typename Result, typename Err>
class expected {
    template <typename T>
    static constexpr int getIndex(T type) {
        if (is_convertible(type, type_v<Result>))
            return 0;
        return 1;
    }

  public:
    using value_type = Result;
    using error_type = Err;

    static_assert(!std::is_reference_v<value_type>, "value_type must not be a reference");
    static_assert(!std::is_reference_v<error_type>, "error_type must not be a reference");

    template <typename T,
              bool same = std::is_convertible_v<value_type, error_type> &&std::is_convertible_v<error_type, value_type>,
              std::enable_if_t<!same> * = nullptr>
    constexpr expected(T &&t) : m_result{std::in_place_index<getIndex(decay_from(t))>, FWD(t)} {}

    template <typename T>
    constexpr expected(value_tag, T &&t) : m_result{std::in_place_index<0>, FWD(t)} {}

    template <typename T>
    constexpr expected(error_tag, T &&t) : m_result{std::in_place_index<1>, FWD(t)} {}

    template <typename R, typename E>
    constexpr expected &operator=(expected<R, E> t) {
        if (t)
            m_result = std::move(t).result();
        else
            m_result = std::move(t).error();
        return *this;
    }

    template <typename T>
    constexpr expected &operator=(T) = delete;

    constexpr operator bool() const noexcept { return m_result.index() == 0; }

    constexpr value_type &&result() && noexcept {
        assert(m_result.index() == 0);
        return std::move(std::get<0>(m_result));
    }

    constexpr const value_type &result() const &noexcept {
        assert(m_result.index() == 0);
        return std::get<0>(m_result);
    }

    constexpr value_type &result() & noexcept {
        assert(m_result.index() == 0);
        return std::get<0>(m_result);
    }

    constexpr error_type &&error() && noexcept {
        assert(m_result.index() == 1);
        return std::move(std::get<1>(m_result));
    }

    constexpr const error_type &error() const &noexcept {
        assert(m_result.index() == 1);
        return std::get<1>(m_result);
    }

    constexpr error_type &error() & noexcept {
        assert(m_result.index() == 1);
        return std::get<1>(m_result);
    }

    constexpr bool is_error() const noexcept { return m_result.index() == 1; }
    constexpr bool is_result() const noexcept { return m_result.index() == 0; }

  private:
    std::variant<value_type, error_type> m_result;
};

} // namespace ltl
