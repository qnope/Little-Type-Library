#pragma once

#include <optional>

#include "BaseIterator.h"

namespace ltl {
template <typename It>
class JoinIterator : public BaseIterator<JoinIterator<It>, It, Nothing, false> {
  friend BaseIterator<JoinIterator, It, Nothing, false>;

public:
  using Container = typename std::iterator_traits<It>::reference;
  using ContainerIterator = decltype(std::declval<Container>().begin());
  using ContainerPtr = AsPointer<Container>;

  using reference = typename std::iterator_traits<ContainerIterator>::reference;
  DECLARE_EVERYTHING_BUT_REFERENCE

  using BaseIterator<JoinIterator<It>, It, Nothing, false>::BaseIterator;

  JoinIterator(It it, It sentinelBegin, It sentinelEnd)
      : BaseIterator<JoinIterator<It>, It, Nothing, false>{
            std::move(it), std::move(sentinelBegin), std::move(sentinelEnd),
            Nothing{}} {
    if (this->m_it == this->m_sentinelEnd)
      return;
    do {
      assignContainerValues();
    } while (m_containerIterator == m_sentinelEndContainer &&
             ++this->m_it != this->m_sentinelEnd);
  }

  JoinIterator &operator++() noexcept {
    if (m_containerIterator != m_sentinelEndContainer) {
      ++m_containerIterator;
    }

    if (m_containerIterator == m_sentinelEndContainer) {
      assert(this->m_it != this->m_sentinelEnd);
      ++this->m_it;
      assignContainerValues();
    }

    return *this;
  }

  JoinIterator &operator--() noexcept {
    if (m_containerIterator == m_sentinelBeginContainer) {
      assert(this->m_it != this->m_sentinelBegin);
      --this->m_it;
      assignContainerValues();
    } else {
      --m_containerIterator;
    }

    return *this;
  }

  reference operator*() const { return *m_containerIterator; }
  pointer operator->() const { return *m_containerIterator; }

  friend std::size_t operator-(const JoinIterator &b, JoinIterator a) {
    std::size_t res = 0;
    while(a != b) {
      ++res;
      ++a;
    }
    return res;
  }

private:
  void assignContainerValues() {
    if (this->m_it != this->m_sentinelEnd) {
      m_currentContainer = ContainerPtr{*this->m_it};
      m_containerIterator = begin(**m_currentContainer);
      m_sentinelBeginContainer = begin(**m_currentContainer);
      m_sentinelEndContainer = end(**m_currentContainer);
    }
  }

  std::optional<ContainerPtr> m_currentContainer;
  ContainerIterator m_containerIterator;
  ContainerIterator m_sentinelBeginContainer;
  ContainerIterator m_sentinelEndContainer;
};

LTL_MAKE_IS_KIND(JoinIterator, is_join_iterator, IsJoinIterator, typename);

struct join_t {};
constexpr join_t join{};
} // namespace ltl
