#pragma once

#include "BaseIterator.h"
#include "Range.h"

namespace ltl {
template <typename ValueType>
struct ValueIterator
    : BaseIterator<ValueIterator<ValueType>, ValueType, Nothing> {
  using reference = ValueType;
  DECLARE_EVERYTHING_BUT_REFERENCE

  ValueIterator() noexcept {
    this->m_sentinelBegin = std::numeric_limits<ValueType>::lowest();
    this->m_sentinelEnd = std::numeric_limits<ValueType>::max();
    this->m_it = this->m_sentinelBegin;
    m_step = 1;
  }

  ValueIterator(ValueType value, ValueType step = 1) noexcept {
    this->m_sentinelBegin = std::numeric_limits<ValueType>::lowest();
    this->m_sentinelEnd = std::numeric_limits<ValueType>::max();
    this->m_it = value;
    m_step = step;
  }

  ValueIterator &operator++() noexcept {
    assert(this->m_it != this->m_sentinelEnd);
    this->m_it += m_step;
    return *this;
  }

  ValueIterator &operator--() noexcept {
    assert(this->m_it != this->m_sentinelBegin);
    this->m_it -= m_step;
    return *this;
  }

  ValueType operator*() const noexcept { return this->m_it; }

  constexpr void advanceUntilNext(...) {}

  ValueType m_step;
};

template <typename ValueType> auto valueRange() {
  return Range{ValueIterator{std::numeric_limits<ValueType>::lowest()},
               ValueIterator{std::numeric_limits<ValueType>::max()}};
}

template <typename ValueType> auto valueRange(ValueType start) {
  return Range{ValueIterator{start},
               ValueIterator{std::numeric_limits<ValueType>::max()}};
}

template <typename ValueType> auto valueRange(ValueType start, ValueType end) {
  return Range{ValueIterator{start}, ValueIterator{end}};
}

template <typename ValueType> auto steppedValueRange(ValueType step) {
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

} // namespace ltl
