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
    using difference_type = long long int;                                                                             \
    using iterator_category = tag

namespace ltl {

template <typename It>
using get_iterator_category = typename std::iterator_traits<It>::iterator_category;

constexpr struct increment_tag_t {
} increment_tag;
constexpr struct decrement_tag_t {
} decrement_tag;

template <typename DerivedIt, typename It>
class BaseIterator :
    public crtp::Comparable<DerivedIt>,
    public crtp::Additionnable<DerivedIt>,
    public crtp::Substractable<DerivedIt>,
    public crtp::PostIncrementable<DerivedIt>,
    public crtp::PostDecrementable<DerivedIt> {
    ENABLE_CRTP(DerivedIt)
  public:
    BaseIterator() = default;
    BaseIterator(It it) noexcept : m_it{std::move(it)} {}

    DerivedIt &operator++() noexcept {
        ++m_it;
        return underlying();
    }

    DerivedIt &operator--() noexcept {
        --m_it;
        return underlying();
    }

    decltype(auto) operator*() const noexcept {
        const DerivedIt &it = underlying();
        return *it.m_it;
    }

    auto operator->() const noexcept {
        const DerivedIt &it = underlying();
        return AsPointer<decltype(*it)>{*it};
    }

    DerivedIt &operator-=(long long int n) noexcept { return underlying() += -n; }

    It m_it{};
};

template <typename Function>
struct WithFunction {
    WithFunction() = default;
    WithFunction(Function f) noexcept : m_function{std::move(f)} {}
    NullableFunction<Function> m_function{};
};

template <typename Derived>
struct IteratorSimpleComparator {
    friend bool operator==(const Derived &a, const Derived &b) noexcept { return a.m_it == b.m_it; }
};

template <typename Derived>
struct IteratorOperationWithDistance {
    ENABLE_CRTP(Derived)

    Derived &operator+=(long long int n) noexcept {
        Derived &it = underlying();
        std::advance(it.m_it, n);
        return it;
    }

    friend std::size_t operator-(const Derived &b, const Derived &a) noexcept { //
        return std::distance(a.m_it, b.m_it);
    }
};

template <typename Derived>
struct IteratorOperationByIterating {
    ENABLE_CRTP(Derived)

    Derived &operator+=(long long int n) noexcept {
        Derived &it = underlying();
        if (n > 0) {
            while (n--)
                ++it;
        } else {
            while (n++)
                --it;
        }
        return it;
    }

    friend std::size_t operator-(const Derived &b, Derived a) {
        std::size_t res = 0;
        for (; a != b; ++res, ++a)
            ;
        return res;
    }
};

template <typename It>
auto safe_advance(It beg, It end, std::size_t n) {
    while (n-- && beg != end)
        ++beg;
    return beg;
}

} // namespace ltl
