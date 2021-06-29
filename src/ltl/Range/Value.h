/**
 * @file Value.h
 */
#pragma once

#include <limits>
#include "BaseIterator.h"
#include "Range.h"

namespace ltl {

/**
 * \defgroup Iterator The iterator group
 * @{
 */

/// \cond

template <typename ValueType>
struct ValueIterator :
    BaseIterator<ValueIterator<ValueType>, ValueType>,
    IteratorSimpleComparator<ValueIterator<ValueType>> {
    using reference = ValueType;
    DECLARE_EVERYTHING_BUT_REFERENCE(std::random_access_iterator_tag);

    ValueIterator() noexcept : BaseIterator<ValueIterator, ValueType>{std::numeric_limits<ValueType>::lowest()} {}

    ValueIterator(ValueType value, ValueType step = 1) noexcept :
        BaseIterator<ValueIterator, ValueType>{std::move(value)}, m_step{std::move(step)} {}

    ValueIterator &operator++() noexcept {
        this->m_it += m_step;
        return *this;
    }

    ValueIterator &operator--() noexcept {
        this->m_it -= m_step;
        return *this;
    }

    ValueIterator &operator+=(long long int n) noexcept {
        this->m_it += static_cast<ValueType>(m_step * n);
        return *this;
    }

    ValueType operator*() const noexcept { return this->m_it; }

    friend std::size_t operator-(const ValueIterator &b, const ValueIterator &a) noexcept {
        return (b.m_it - a.m_it) / (b.m_step);
    }

    ValueType m_step{static_cast<ValueType>(1)};
};

/// \endcond

template <typename ValueType>
/**
 * @brief valueRange - returns the full range value of type ValueType
 */
auto valueRange() {
    return Range{ValueIterator{std::numeric_limits<ValueType>::lowest()},
                 ValueIterator{std::numeric_limits<ValueType>::max()}};
}

template <typename ValueType>
/**
 * @brief valueRange
 * @param start
 */
auto valueRange(ValueType start) {
    return Range{ValueIterator{start}, ValueIterator{std::numeric_limits<ValueType>::max()}};
}

template <typename ValueType>
/**
 * @brief valueRange
 * @param start
 * @param end
 */
auto valueRange(ValueType start, ValueType end) {
    return Range{ValueIterator{start}, ValueIterator{end}};
}

/// \cond

template <typename ValueType>
auto steppedValueRange(ValueType step) {
    auto begin = ValueIterator{std::numeric_limits<ValueType>::lowest(), step};
    auto end = ValueIterator{std::numeric_limits<ValueType>::max(), step};
    return Range{begin, end};
}

template <typename ValueType>
auto steppedValueRange(ValueType start, ValueType step) {
    auto begin = ValueIterator{start, step};
    auto end = ValueIterator{std::numeric_limits<ValueType>::max(), step};
    return Range{begin, end};
}

template <typename ValueType>
auto steppedValueRange(ValueType start, ValueType end, ValueType step) {
    auto begin = ValueIterator{start, step};
    auto _end = ValueIterator{end, step};
    return Range{begin, _end};
}

/// \endcond

/**
 * @brief ints - returns an "infinite" list beginning at 0
 */
inline auto ints() { return valueRange<int>(0); }

/**
 * @brief ints - returns a finite list beginning at 0 finishing at size - 1
 * @param size
 */
inline auto ints(int size) { return valueRange<int>(0, size); }

/// @}

} // namespace ltl
