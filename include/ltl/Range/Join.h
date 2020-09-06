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

  private:
    struct ContainerWrapper {
        ContainerWrapper(ContainerPtr ptr) noexcept : ptr{std::move(ptr)} { computeIterators(0); }

        ContainerWrapper(const ContainerWrapper &wrapper) noexcept : ptr{wrapper.ptr} {
            computeIterators(std::distance(wrapper.beginSentinel, wrapper.current));
        }

        ContainerWrapper(ContainerWrapper &&wrapper) noexcept : ptr{std::move(wrapper.ptr)} {
            computeIterators(std::distance(wrapper.beginSentinel, wrapper.current));
        }

        ContainerWrapper &operator=(ContainerWrapper wrapper) noexcept {
            ptr = std::move(wrapper.ptr);
            computeIterators(std::distance(wrapper.beginSentinel, wrapper.current));
            return *this;
        }

        void computeIterators(long long int distance) {
            current = std::next(begin(*ptr), distance);
            beginSentinel = begin(*ptr);
            endSentinel = end(*ptr);
        }

        ContainerPtr ptr;
        ContainerIterator current;
        ContainerIterator beginSentinel;
        ContainerIterator endSentinel;
    };

  public:
    using BaseIterator<JoinIterator<It>, It, Nothing, false, false>::BaseIterator;

    JoinIterator(It it, It sentinelBegin, It sentinelEnd) :
        BaseIterator<JoinIterator<It>, It, Nothing, false, false>{std::move(it), std::move(sentinelBegin),
                                                                  std::move(sentinelEnd), Nothing{}} {
        while (!assignContainerValues())
            ++this->m_it;
    }

    JoinIterator &operator++() noexcept {
        if (m_wrapper->current != m_wrapper->endSentinel) {
            ++m_wrapper->current;
        }

        if (m_wrapper->current == m_wrapper->endSentinel) {
            assert(this->m_it != this->m_sentinelEnd);
            do {
                ++this->m_it;
            } while (!assignContainerValues());
        }

        return *this;
    }

    JoinIterator &operator--() noexcept {
        if (m_wrapper->current == m_wrapper->beginSentinel) {
            assert(this->m_it != this->m_sentinelBegin);
            do {
                --this->m_it;
            } while (!assignContainerValues());
            m_wrapper->current = std::prev(m_wrapper->endSentinel);
        } else {
            --m_wrapper->current;
        }

        return *this;
    }

    reference operator*() const { return *m_wrapper->current; }
    pointer operator->() const { return *m_wrapper->current; }

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
            return a.m_wrapper->current == b.m_wrapper->current;
        }
        return false;
    }

  private:
    [[nodiscard]] bool assignContainerValues() {
        if (this->m_it == this->m_sentinelEnd) {
            return true;
        }
        m_wrapper.emplace(ContainerPtr{*this->m_it});
        return m_wrapper->current != m_wrapper->endSentinel;
    }

    std::optional<ContainerWrapper> m_wrapper;
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
