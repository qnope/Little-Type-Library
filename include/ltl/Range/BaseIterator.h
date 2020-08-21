#pragma once

#include <algorithm>
#include <cassert>
#include <utility>

#include "../crtp.h"
#include "../traits.h"
#include "AsPointer.h"
#include "NullableFunction.h"

#define DECLARE_EVERYTHING_BUT_REFERENCE(tag)                                                                          \
    using pointer = ::ltl::AsPointer<reference>;                                                                       \
    using value_type = std::decay_t<reference>;                                                                        \
    using difference_type = std::size_t;                                                                               \
    using iterator_category = tag;

namespace ltl {

template <typename DerivedIt, typename It, typename Function, bool CreateMinusOperator = true,
          bool CreateEqualOperator = true>
class BaseIterator :
    public PostIncrementable<DerivedIt>,
    public PostDecrementable<DerivedIt>,
    public Comparable<DerivedIt>,
    public Additionnable<DerivedIt>,
    public Substractable<DerivedIt> {
    ENABLE_CRTP(DerivedIt)
  public:
    BaseIterator() = default;

    BaseIterator(It it) : m_it{it}, m_sentinelBegin{it}, m_sentinelEnd{it} {}

    BaseIterator(It it, It sentinelBegin, It sentinelEnd, Function function) noexcept :
        m_it{std::move(it)}, m_sentinelBegin{std::move(sentinelBegin)}, m_sentinelEnd{std::move(sentinelEnd)},
        m_function{std::move(function)} {}

    template <bool create = CreateEqualOperator, typename = std::enable_if_t<create>>
    friend bool operator==(const DerivedIt &a, const DerivedIt &b) noexcept {
        return a.m_it == b.m_it;
    }

    DerivedIt &operator++() noexcept {
        assert(m_it != m_sentinelEnd);
        ++m_it;
        return underlying();
    }

    DerivedIt &operator--() noexcept {
        assert(m_it != m_sentinelBegin);
        --m_it;
        return underlying();
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

    template <bool createMinus = CreateMinusOperator, typename = std::enable_if_t<createMinus>>
    friend std::size_t operator-(const DerivedIt &b, const DerivedIt &a) noexcept {
        return std::distance(a.m_it, b.m_it);
    }

  protected:
    It m_it{};
    It m_sentinelBegin{};
    It m_sentinelEnd{};
    NullableFunction<Function> m_function{};
};
} // namespace ltl
