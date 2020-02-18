#pragma once

#include "../ltl.h"
#include "BaseIterator.h"

namespace ltl {
template <typename It, typename Predicate>
class FilterIterator
    : public BaseIterator<FilterIterator<It, Predicate>, It, Predicate> {
  friend BaseIterator<FilterIterator, It, Predicate>;

public:
  using reference = typename std::iterator_traits<It>::reference;
  DECLARE_EVERYTHING_BUT_REFERENCE

  using BaseIterator<FilterIterator<It, Predicate>, It,
                     Predicate>::BaseIterator;

  FilterIterator(It it, It sentinelBegin, It sentinelEnd, Predicate function)
      : BaseIterator<FilterIterator<It, Predicate>, It, Predicate>{
            std::move(it), std::move(sentinelBegin), std::move(sentinelEnd),
            std::move(function)} {
    this->m_it =
        std::find_if(this->m_it, this->m_sentinelEnd, this->m_function);
  }

  FilterIterator &operator++() noexcept {
    this->m_it = std::find_if(std::next(this->m_it), this->m_sentinelEnd,
                              this->m_function);
    return *this;
  }

  FilterIterator &operator--() noexcept {
    --this->m_it;
    while (!this->m_function(*this->m_it) &&
           this->m_it != this->m_sentinelBegin) {
      --this->m_it;
    }
    assert(this->m_function(*this->m_it));
    return *this;
  }
}; // namespace ltl

template <typename F> struct FilterType { F f; };
template <typename F> auto filter(F &&f) {
  return FilterType<std::decay_t<F>>{FWD(f)};
}

LTL_MAKE_IS_KIND(FilterIterator, is_filter_iterator, IsFilterIterator,
                 typename);
LTL_MAKE_IS_KIND(FilterType, is_filter_type, IsFilterType, typename);

} // namespace ltl
