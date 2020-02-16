#pragma once

#include "../ltl.h"
#include "BaseIterator.h"

namespace ltl {
template <typename It>
class TakerIterator : public BaseIterator<TakerIterator<It>, It, Nothing> {
  friend BaseIterator<TakerIterator, It, Nothing>;

public:
  using reference = typename std::iterator_traits<It>::reference;
  DECLARE_EVERYTHING_BUT_REFERENCE

  TakerIterator() = default;

  TakerIterator(It begin, It sentinelBegin, It sentinelEnd, std::size_t n)
      : BaseIterator<TakerIterator<It>, It, Nothing>{std::move(begin),
                                                     std::move(sentinelBegin),
                                                     std::move(sentinelEnd),
                                                     Nothing{}},
        m_n{n} {
    advanceUntilNext(IncrementTag{});
  }

  TakerIterator(It begin)
      : BaseIterator<TakerIterator<It>, It, Nothing>{std::move(begin)} {}

private:
  template <typename Tag> void advanceUntilNext(Tag) noexcept {
    if_constexpr(type_v<Tag> == type_v<IncrementTag>) {
      if (m_n == 0) {
        this->m_it = this->m_sentinelEnd;
      } else
        --m_n;
    }

    else {
      ++m_n;
    }
  }

  std::size_t m_n{0};
};

LTL_MAKE_IS_KIND(TakerIterator, is_taker_iterator, IsTakerIterator, typename);

struct TakerType {
  std::size_t n;
};
inline auto take_n(std::size_t n) { return TakerType{n}; }

} // namespace ltl
