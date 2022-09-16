/**
 * @file Reverse.h
 */
#pragma once

#include "Range.h"
#include "BaseIterator.h"

namespace ltl {

/**
 * \defgroup Iterator The iterator group
 * @{
 */

using std::begin;
using std::end;

/// \cond

template <typename It>
class ReverseIterator :
    public BaseIterator<ReverseIterator<It>, It>,
    public IteratorOperationByIterating<ReverseIterator<It>> {
  public:
    using reference = typename std::iterator_traits<It>::reference;

    static_assert(std::is_base_of_v<std::bidirectional_iterator_tag, get_iterator_category<It>>,
                  "It must be a reversible iterator to use a reversed range");

    DECLARE_EVERYTHING_BUT_REFERENCE(std::random_access_iterator_tag);

    ReverseIterator() = default;

    ReverseIterator(It it, It begin, bool isSentinel) noexcept :
        BaseIterator<ReverseIterator, It>{std::move(it)}, //
        m_sentinelBegin{std::move(begin)},                //
        m_isSentinel{isSentinel} {}

    ReverseIterator &operator++() {
        assert(!(this->m_it == this->m_sentinelBegin && m_isSentinel));
        if (this->m_it == this->m_sentinelBegin) {
            m_isSentinel = true;
        } else {
            --this->m_it;
        }
        return *this;
    }

    ReverseIterator &operator--() {
        if (m_isSentinel) {
            m_isSentinel = false;
        } else {
            ++this->m_it;
        }
        return *this;
    }

    friend bool operator==(const ReverseIterator &a, const ReverseIterator &b) noexcept {
        return a.m_isSentinel == b.m_isSentinel && a.m_it == b.m_it;
    }

  private:
    It m_sentinelBegin;
    bool m_isSentinel;
};

template <typename It, bool reversable>
struct WithSentinelImpl;

template <typename It>
struct WithSentinelImpl<It, false> {
    using reverse_iterator = empty_t;

    WithSentinelImpl() = default;

    template <typename T>
    WithSentinelImpl(T &&, It e) noexcept : m_sentinelEnd{std::move(e)} {}

    empty_t reverse(const It &) const noexcept { return {}; }

    It m_sentinelEnd{};
};

template <typename It>
struct WithSentinelImpl<It, true> {
    using reverse_iterator = ReverseIterator<It>;

    WithSentinelImpl() = default;
    WithSentinelImpl(It b, It e) noexcept : m_sentinelBegin{b, b, true}, m_sentinelEnd{std::move(e)} {}

    ReverseIterator<It> reverse(const It &it) const noexcept {
        return {it, m_sentinelBegin.m_it, m_sentinelBegin.m_it == m_sentinelEnd};
    }

    ReverseIterator<It> m_sentinelBegin{};
    It m_sentinelEnd{};
};

template <typename It, bool reversable = std::is_base_of_v<std::bidirectional_iterator_tag, get_iterator_category<It>>>
using WithSentinel = WithSentinelImpl<It, reversable>;

struct reverse_t {};

/// \endcond

/**
 * @brief reversed - Used to iterate on a reversed array
 *
 * @code
 *  std::vector<int> array = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
 *
 *  // reversed_array = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
 *  auto reversed_array = array | ltl::reversed;
 * @endcode
 */
constexpr reverse_t reversed;

/// \cond

template <>
struct is_chainable_operation<reverse_t> : true_t {};

template <typename T1, requires_f(IsIterableRef<T1>)>
constexpr decltype(auto) operator|(T1 &&a, reverse_t) {
    auto b = begin(FWD(a));
    auto e = end(FWD(a));
    using It = ReverseIterator<decltype(b)>;

    if (b == e)
        return Range<It>{{e, e, true}, {e, e, true}};

    auto rb = std::prev(e);
    return Range<It>{{rb, b, false}, {b, b, true}};
}

/// \endcond

/// @}

} // namespace ltl
