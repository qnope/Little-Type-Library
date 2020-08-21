#pragma once

#include <optional>

#include "Range.h"
#include "BaseIterator.h"

namespace ltl {
template <typename It>
class JoinIterator : public BaseIterator<JoinIterator<It>, It, Nothing, false, false> {
    friend BaseIterator<JoinIterator, It, Nothing, false, false>;

  public:
    using Container = typename std::iterator_traits<It>::reference;
    using ContainerIterator = decltype(std::declval<Container>().begin());
    using ContainerPtr = AsPointer<Container>;

    using reference = typename std::iterator_traits<ContainerIterator>::reference;
    DECLARE_EVERYTHING_BUT_REFERENCE(typename std::iterator_traits<It>::iterator_category)

    using BaseIterator<JoinIterator<It>, It, Nothing, false, false>::BaseIterator;

    JoinIterator(It it, It sentinelBegin, It sentinelEnd) :
        BaseIterator<JoinIterator<It>, It, Nothing, false, false>{std::move(it), std::move(sentinelBegin),
                                                                  std::move(sentinelEnd), Nothing{}} {
        while (!assignContainerValues())
            ++this->m_it;
    }

    JoinIterator &operator++() noexcept {
        if (m_containerIterator != m_sentinelEndContainer) {
            ++m_containerIterator;
        }

        if (m_containerIterator == m_sentinelEndContainer) {
            assert(this->m_it != this->m_sentinelEnd);
            do {
                ++this->m_it;
            } while (!assignContainerValues());
        }

        return *this;
    }

    JoinIterator &operator--() noexcept {
        if (m_containerIterator == m_sentinelBeginContainer) {
            assert(this->m_it != this->m_sentinelBegin);
            do {
                --this->m_it;
            } while (!assignContainerValues());
            m_containerIterator = m_sentinelEndContainer;
            --m_containerIterator;
        } else {
            --m_containerIterator;
        }

        return *this;
    }

    reference operator*() const { return *m_containerIterator; }
    pointer operator->() const { return *m_containerIterator; }

    friend std::size_t operator-(const JoinIterator &b, JoinIterator a) {
        std::size_t res = 0;
        while (a != b) {
            ++res;
            ++a;
        }
        return res;
    }

    friend bool operator==(const JoinIterator &a, const JoinIterator &b) noexcept {
        if (a.m_it == b.m_it) {
            if (a.m_it == a.m_sentinelEnd)
                return true;
            return a.m_containerIterator == b.m_containerIterator;
        }
        return false;
    }

  private:
    [[nodiscard]] bool assignContainerValues() {
        if (this->m_it == this->m_sentinelEnd) {
            return true;
        }
        m_currentContainer = ContainerPtr{*this->m_it};
        m_containerIterator = begin(**m_currentContainer);
        m_sentinelBeginContainer = begin(**m_currentContainer);
        m_sentinelEndContainer = end(**m_currentContainer);
        return m_containerIterator != m_sentinelEndContainer;
    }

    std::optional<ContainerPtr> m_currentContainer;
    ContainerIterator m_containerIterator;
    ContainerIterator m_sentinelBeginContainer;
    ContainerIterator m_sentinelEndContainer;
};

struct join_t {};
constexpr join_t join{};

template <>
struct is_chainable_operation<join_t> : true_t {};

template <typename T1, requires_f(IsIterableRef<T1>)>
constexpr decltype(auto) operator|(T1 &&a, join_t) {
    using std::begin;
    using std::end;
    using it = decltype(begin(FWD(a)));
    return Range{JoinIterator<it>{begin(FWD(a)), begin(FWD(a)), end(FWD(a))}, JoinIterator<it>{end(FWD(a))}};
}
} // namespace ltl
