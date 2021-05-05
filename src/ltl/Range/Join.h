#pragma once

#include <optional>

#include "Range.h"
#include "Reverse.h"
#include "BaseIterator.h"

namespace ltl {
template <typename It>
class JoinIterator :
    public BaseIterator<JoinIterator<It>, It>,
    public WithSentinel<It>,
    public IteratorOperationByIterating<JoinIterator<It>> {
  public:
    using Container = typename std::iterator_traits<It>::reference;
    using ContainerIterator = decltype(std::declval<Container>().begin());
    using ContainerPtr = AsPointer<Container>;

    using reference = typename std::iterator_traits<ContainerIterator>::reference;
    DECLARE_EVERYTHING_BUT_REFERENCE(get_iterator_category<It>);

  public:
    JoinIterator() = default;

    JoinIterator(It it, It sentinelBegin, It sentinelEnd) :
        BaseIterator<JoinIterator, It>{std::move(it)}, //
        WithSentinel<It>{std::move(sentinelBegin), std::move(sentinelEnd)} {
        while (!assignContainerValues())
            ++this->m_it;
    }

    JoinIterator &operator++() noexcept {
        if (m_current != m_end) {
            ++m_current;
        }

        if (m_current == m_end) {
            assert(this->m_it != this->m_sentinelEnd);
            do {
                ++this->m_it;
            } while (!assignContainerValues());
        }
        return *this;
    }

    JoinIterator &operator--() noexcept {
        if (m_current == 0) {
            assert(this->reverse(this->m_it) != this->m_sentinelBegin);
            do {
                --this->m_it;
            } while (!assignContainerValues());
            m_current = m_end - 1;
        } else {
            --m_current;
        }
        return *this;
    }

    reference operator*() const { return *std::next(begin(**const_cast<JoinIterator &>(*this).m_ptr), m_current); }

    friend bool operator==(const JoinIterator &a, const JoinIterator &b) noexcept {
        if (a.m_it == b.m_it) {
            if (a.m_it == a.m_sentinelEnd)
                return true;
            return a.m_current == b.m_current;
        }
        return false;
    }

  private:
    [[nodiscard]] bool assignContainerValues() {
        if (this->m_it == this->m_sentinelEnd) {
            return true;
        }
        m_ptr.emplace(*this->m_it);
        m_current = 0;
        m_end = std::distance(begin(**m_ptr), end(**m_ptr));
        return m_end > 0;
    }

    std::optional<ContainerPtr> m_ptr;
    long long int m_current{0};
    long long int m_end{0};
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
    return Range{JoinIterator<it>{begin(FWD(a)), begin(FWD(a)), end(FWD(a))},
                 JoinIterator<it>{end(FWD(a)), begin(FWD(a)), end(FWD(a))}};
}
} // namespace ltl
