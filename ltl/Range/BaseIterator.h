#pragma once

#include <utility>

#include "../crtp.h"
#include "AsPointer.h"
#include "NullableFunction.h"

#define DECLARE_EVERYTHING_BUT_REFERENCE                                       \
  using pointer = AsPointer<reference>;                                        \
  using value_type = std::decay_t<reference>;                                  \
  using difference_type = std::size_t;                                         \
  using iterator_category = std::random_access_iterator_tag;

namespace ltl {

struct IncrementTag {};
struct DecrementTag {};

template <typename DerivedIt, typename It, typename Function>
class BaseIterator : public PostIncrementable<DerivedIt>,
                     public PostDecrementable<DerivedIt>,
                     public Comparable<DerivedIt>,
                     public Additionnable<DerivedIt>,
                     public Substractable<DerivedIt> {
  ENABLE_CRTP(DerivedIt)
public:
  BaseIterator() = default;

  BaseIterator(It it) : m_it{it}, m_sentinelBegin{it}, m_sentinelEnd{it} {}

  BaseIterator(It it, It sentinelBegin, It sentinelEnd,
               Function function) noexcept
      : m_it{std::move(it)}, m_sentinelBegin{std::move(sentinelBegin)},
        m_sentinelEnd{std::move(sentinelEnd)}, m_function{std::move(function)} {
    DerivedIt &derived = underlying();
    derived.advanceUntilNext(IncrementTag{});
  }

  bool operator==(const DerivedIt &it) const noexcept {
    return underlying().m_it == it.m_it;
  }

  DerivedIt &operator++() noexcept {
    DerivedIt &it = underlying();
    assert(it.m_it != it.m_sentinelEnd);
    ++it.m_it;
    it.advanceUntilNext(IncrementTag{});

    return it;
  }

  DerivedIt &operator--() noexcept {
    DerivedIt &it = underlying();
    assert(it.m_it != it.m_sentinelBegin);
    --it.m_it;
    it.advanceUntilNext(DecrementTag{});
    return it;
  }

  decltype(auto) operator*() const noexcept {
    const DerivedIt &it = underlying();
    assert(it.m_it != it.m_sentinelEnd);
    return *it.m_it;
  }

  auto operator-> () const noexcept {
    const DerivedIt &it = underlying();
    return AsPointer<decltype(*it)>{*it};
  }

  DerivedIt &operator+=(long long int n) noexcept {
    DerivedIt &it = underlying();
    if (n > 0) {
      while (n--)
        ++it;
    }

    else {
      while (n++)
        --it;
    }
    return it;
  }

  DerivedIt &operator-=(long long int n) noexcept {
    DerivedIt &it = underlying();
    if (n > 0) {
      while (n--)
        --it;
    }

    else {
      while (n++)
        ++it;
    }
    return it;
  }

  friend std::size_t operator-(const DerivedIt &b, DerivedIt a) noexcept {
    std::size_t d{0};
    while (a != b) {
      ++d;
      ++a;
    }
    return d;
  }

protected:
  It m_it{};
  It m_sentinelBegin{};
  It m_sentinelEnd{};
  NullableFunction<Function> m_function{};
};
} // namespace ltl
