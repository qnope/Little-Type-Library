/**
 * @file Repeater.h
 */
#pragma once

#include "Range.h"
#include "BaseIterator.h"
#include <variant>
#include <optional>

namespace ltl {

/**
 * \defgroup Iterator The iterator group
 * @{
 */

/// \cond
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

    auto begin() const noexcept {
        if (m_value)
            return repeater_iterator<T>{m_number, &*m_value};
        return repeater_iterator<T>{};
    }
    auto end() const noexcept { return repeater_iterator<T>{m_number}; }

  private:
    std::optional<T> m_value;
    long long int m_number{};
};

/// \endcond

template <typename T>
/**
 * @brief make_empty_range - returns an empty range of T
 */
auto make_empty_range() noexcept {
    return RepeaterRange<T>{};
}

template <typename T>
/**
 * @brief make_repeater_range - returns a range of number time value
 *
 * @code
 *  array = [10, 10, 10, 10, 10];
 *  auto array = make_repeater_range(10, 5);
 * @endcode
 * @param value
 * @param number
 */
auto make_repeater_range(T value, long long int number) noexcept {
    return RepeaterRange<T>{std::move(value), number};
}

template <typename T>
/**
 * @brief make_mono_element_range - returns the value wrapped into a range of one element
 *
 * @code
 *  // array = [10]
 *  auto array = make_mono_element_range(10);
 * @endcode
 *
 * Note : It is equivalent to `ltl::make_repeater_range(value, 1);`
 * @param value
 */
auto make_mono_element_range(T value) noexcept {
    return make_repeater_range(std::move(value), 1);
}

template <typename T>
/**
 * @brief yield_if - Returns a range of one or zero value depending on the boolean
 *
 * @code
 *  std::vector<int> array = {0, 1, 2, 3, 4, 5};
 *  auto is_odd = [](auto x) {return x % 2 == 1;};
 *  auto yielder = [&](auto x) {return ltl::yield_if(is_odd(x), x);};
 *  // equivalent to result = array | ltl::filter(is_odd);
 *  auto result = array >> ltl::map(yielder);
 * @endcode
 * @param toYield
 * @param value
 */
auto yield_if(bool toYield, T value) noexcept {
    auto n = static_cast<long long int>(toYield);
    return make_repeater_range(std::move(value), n);
}

template <typename Nullable>
/**
 * @brief yield - Returns a range of one or zero value depending on the nullable type
 *
 * @code
 *  struct Person {
 *      std::string name;
 *  };
 *  std::vector<std::optional<Person>> persons;
 *  persons.push_back(Person{"Bill"});
 *  persons.push_back(std::nullopt);
 *  persons.push_back(Person{"John"});
 *
 *  // persons2 = [Person{"Bill"}, Person{"John"}];
 *  std::vector<Person> persons2 = persons >> ltl::map([](auto x) {return ltl::yield(std::move(x));};
 * @endcode
 * @param x
 */
auto yield(Nullable x) noexcept {
    if (x) {
        return make_mono_element_range(std::move(*x));
    } else {
        return make_empty_range<remove_cvref_t<decltype(*std::declval<Nullable>())>>();
    }
}

/// @}

} // namespace ltl
