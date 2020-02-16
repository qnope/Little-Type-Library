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

private:
  template <typename Tag> void advanceUntilNext(Tag) noexcept {
    if_constexpr(type_v<Tag> == type_v<IncrementTag>) {
      while (this->m_it != this->m_sentinelEnd &&
             !this->m_function(*this->m_it)) {
        ++this->m_it;
      }
    }

    else {
      while (this->m_it != this->m_sentinelBegin &&
             !this->m_function(*this->m_it)) {
        --this->m_it;
      }
      assert(this->m_function(*this->m_it));
    }
  }
};

template <typename F> struct FilterType { F f; };
template <typename F> auto filter(F &&f) {
  return FilterType<std::decay_t<F>>{FWD(f)};
}

LTL_MAKE_IS_KIND(FilterIterator, is_filter_iterator, IsFilterIterator,
                 typename);
LTL_MAKE_IS_KIND(FilterType, is_filter_type, IsFilterType, typename);

} // namespace ltl
