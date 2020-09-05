#pragma once

#include "Range.h"
#include "BaseIterator.h"
#include <optional>

namespace ltl {

template <typename T>
class repeater_iterator :
    public PostIncrementable<repeater_iterator<T>>,
    public PostDecrementable<repeater_iterator<T>>,
    public Comparable<repeater_iterator<T>>,
    public Additionnable<repeater_iterator<T>>,
    public Substractable<repeater_iterator<T>> {
  public:
    using reference = const T &;

    DECLARE_EVERYTHING_BUT_REFERENCE(std::random_access_iterator_tag)

    repeater_iterator() noexcept {};

    repeater_iterator(long long int n, T value) noexcept : m_current{0}, m_end{n}, m_value{std::move(value)} {}

    repeater_iterator(long long int n) noexcept : m_current{n} {}

    repeater_iterator(const repeater_iterator &) = default;
    repeater_iterator(repeater_iterator &&) = default;

    repeater_iterator &operator=(repeater_iterator it) noexcept {
        m_current = it.m_current;
        m_end = it.m_end;
        m_value = std::move(it.m_value);
        return *this;
    }

    reference operator*() const noexcept {
        assert(m_value);
        assert(m_current != m_end);
        return *m_value;
    }

    pointer operator->() const noexcept {
        assert(m_value);
        assert(m_current != m_end);

        return {*m_value};
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
    std::optional<T> m_value{};
};

template <typename T>
auto make_repeater_range(T value, long long int number) {
    return Range{repeater_iterator<T>{number, std::move(value)}, repeater_iterator<T>{number}};
}

} // namespace ltl
