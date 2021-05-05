#pragma once

#include "Range.h"
#include "BaseIterator.h"
#include <variant>
#include <optional>

namespace ltl {

template <typename T>
class repeater_iterator :
    public crtp::PostIncrementable<repeater_iterator<T>>,
    public crtp::PostDecrementable<repeater_iterator<T>>,
    public crtp::Comparable<repeater_iterator<T>>,
    public crtp::Additionnable<repeater_iterator<T>>,
    public crtp::Substractable<repeater_iterator<T>> {
  public:
    using reference = const T &;

    DECLARE_EVERYTHING_BUT_REFERENCE(std::random_access_iterator_tag);

    repeater_iterator() noexcept {};

    repeater_iterator(long long int n, const T *value) noexcept : m_current{0}, m_end{n}, m_value{value} {}

    repeater_iterator(long long int n) noexcept : m_current{n} {}

    reference operator*() const noexcept {
        assert(m_value);
        assert(m_current != m_end);
        return *m_value;
    }

    repeater_iterator &operator+=(long long int n) noexcept {
        assert(m_value);
        assert(0 <= m_current + n && m_current + n <= m_end);
        m_current += n;
        return *this;
    }

    repeater_iterator &operator-=(long long int n) noexcept {
        assert(m_value);
        assert(0 <= m_current - n && m_current - n <= m_end);
        m_current -= n;
        return *this;
    }

    repeater_iterator &operator++() noexcept { return (*this += 1); }
    repeater_iterator &operator--() noexcept { return (*this -= 1); }

    friend bool operator==(const repeater_iterator &a, const repeater_iterator &b) noexcept {
        return a.m_current == b.m_current;
    }

    friend std::size_t operator-(const repeater_iterator &b, const repeater_iterator &a) noexcept {
        return b.m_current - a.m_current;
    }

  private:
    long long int m_current{};
    long long int m_end{};
    const T *m_value{};
};

template <typename T>
class RepeaterRange : public AbstractRange<RepeaterRange<T>> {
  public:
    RepeaterRange() = default;

    RepeaterRange(T value, long long int number) noexcept :
        m_value{std::move(value)}, //
        m_number{number} {}

    RepeaterRange(RepeaterRange &&) = default;
    RepeaterRange(const RepeaterRange &) = default;
    RepeaterRange &operator=(RepeaterRange &&) = default;
    RepeaterRange &operator=(const RepeaterRange &) = default;

    auto begin() const noexcept {
        if (m_value)
            return repeater_iterator<T>{m_number, &*m_value};
        return repeater_iterator<T>{};
    }
    auto end() const noexcept { return repeater_iterator<T>{m_number}; }

  private:
    std::optional<T> m_value;
    long long int m_number{};
}; // namespace ltl

template <typename T>
auto make_empty_range() noexcept {
    return RepeaterRange<T>{};
}

template <typename T>
auto make_repeater_range(T value, long long int number) noexcept {
    return RepeaterRange<T>{std::move(value), number};
}

template <typename T>
auto make_mono_element_range(T value) noexcept {
    return make_repeater_range(std::move(value), 1);
}

template <typename T>
auto yield_if(bool toYield, T value) noexcept {
    auto n = static_cast<long long int>(toYield);
    return make_repeater_range(std::move(value), n);
}

} // namespace ltl
