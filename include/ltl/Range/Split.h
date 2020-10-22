#pragma once

#include "Range.h"
#include "Reverse.h"

namespace ltl {
using std::begin;
using std::end;

template <typename It, typename AdvanceIt, typename Dereference, std::size_t ElementCountToSkip>
class SplitIterator :
    public BaseIterator<SplitIterator<It, AdvanceIt, Dereference, ElementCountToSkip>, It>,
    public WithSentinel<It>,
    public IteratorOperationByIterating<SplitIterator<It, AdvanceIt, Dereference, ElementCountToSkip>>,
    public IteratorSimpleComparator<SplitIterator<It, AdvanceIt, Dereference, ElementCountToSkip>> {
  public:
    using reference =
        decltype(fast_invoke(std::declval<Dereference>(), std::declval<const It &>(), std::declval<const It &>()));
    DECLARE_EVERYTHING_BUT_REFERENCE(get_iterator_category<It>);

    SplitIterator() = default;

    SplitIterator(It it, It sentinelBegin, It sentinelEnd, AdvanceIt advanceIt, Dereference dereference) :
        BaseIterator<SplitIterator, It>{std::move(it)}, //
        WithSentinel<It>{sentinelBegin, sentinelEnd},   //
        m_advance{advanceIt},                           //
        m_dereference{dereference},                     //
        m_nextIterator{m_advance(increment_tag, this->m_it, this->m_sentinelEnd)} {
        if_constexpr(type_from(m_previousIterator) != type_v<Nothing>) {
            if (it != sentinelBegin) {
                SplitIterator i{sentinelBegin, sentinelBegin, sentinelEnd, advanceIt, dereference};
                for (; i != *this; ++i)
                    ;
                m_previousIterator = i.m_previousIterator;
            }
        }
    }

    reference operator*() const noexcept { return m_dereference(this->m_it, this->m_nextIterator); }

    SplitIterator &operator++() noexcept {
        m_previousIterator = this->reverse(this->m_it);
        this->m_it = safe_advance(m_nextIterator, this->m_sentinelEnd, ElementCountToSkip);
        m_nextIterator = m_advance(increment_tag, this->m_it, this->m_sentinelEnd);
        return *this;
    }

    SplitIterator &operator--() noexcept {
        if (this->m_it != this->m_nextIterator)
            m_nextIterator = std::prev(this->m_it, ElementCountToSkip);
        this->m_it = m_previousIterator.m_it;
        m_previousIterator = m_advance(decrement_tag, m_previousIterator, this->m_sentinelBegin);
        return *this;
    }

  private:
    NullableFunction<AdvanceIt> m_advance;
    NullableFunction<Dereference> m_dereference;
    typename WithSentinel<It>::reverse_iterator m_previousIterator;
    It m_nextIterator;
};

template <typename T>
struct split_with_t {
    T object;
};

struct chunk_t {
    std::size_t n;
};

template <typename T>
struct is_chainable_operation<split_with_t<T>> : true_t {};

template <>
struct is_chainable_operation<chunk_t> : true_t {};

template <typename T>
auto split(T t) {
    return split_with_t<T>{std::move(t)};
}

chunk_t chunks(std::size_t n) { return {n}; }

template <typename F>
struct GroupByType {
    F f;
};
template <typename F>
auto group_by(F &&f) {
    return GroupByType<std::decay_t<F>>{FWD(f)};
}

template <typename F>
struct is_chainable_operation<GroupByType<F>> : true_t {};

namespace details {
inline auto dereference_to_range = [](auto it, auto end) { return Range<decltype(it)>{std::move(it), std::move(end)}; };
using DereferenceToRange = decltype(dereference_to_range);
} // namespace details

template <typename T1, typename T2, requires_f(IsIterableRef<T1>)>
decltype(auto) operator|(T1 &&a, split_with_t<T2> b) {
    using it = decltype(begin(FWD(a)));
    auto advance = ltl::overloader{[object = b.object](increment_tag_t, const auto &beg, const auto &end) { //
                                       return std::find(beg, end, object);
                                   },
                                   [object = b.object](decrement_tag_t, auto beg, const auto &end) { //
                                       beg = safe_advance(beg, end, 2);
                                       return std::prev(std::find(beg, end, object));
                                   }};
    using Advance = decltype(advance);
    return Range{SplitIterator<it, Advance, details::DereferenceToRange, 1>{begin(FWD(a)), begin(FWD(a)), end(FWD(a)),
                                                                            advance, details::dereference_to_range},
                 SplitIterator<it, Advance, details::DereferenceToRange, 1>{end(FWD(a)), begin(FWD(a)), end(FWD(a)),
                                                                            advance, details::dereference_to_range}};
}

template <typename T1, requires_f(IsIterableRef<T1>)>
decltype(auto) operator|(T1 &&a, chunk_t b) {
    using it = decltype(begin(FWD(a)));
    auto advance = [n = b.n](auto &&, const auto &beg, const auto &end) { return safe_advance(beg, end, n); };
    using Advance = decltype(advance);
    return Range{SplitIterator<it, Advance, details::DereferenceToRange, 0>{begin(FWD(a)), begin(FWD(a)), end(FWD(a)),
                                                                            advance, details::dereference_to_range},
                 SplitIterator<it, Advance, details::DereferenceToRange, 0>{end(FWD(a)), begin(FWD(a)), end(FWD(a)),
                                                                            advance, details::dereference_to_range}};
}

template <typename T1, typename F, requires_f(IsIterableRef<T1>)>
constexpr decltype(auto) operator|(T1 &&a, GroupByType<F> b) {
    using it = decltype(begin(FWD(a)));

    auto dereference = [f = b.f](auto it, auto end) {
        decltype(auto) value = ltl::fast_invoke(f, *it);
        return ltl::tuple_t<decltype(value), Range<decltype(it)>>{value, {std::move(it), std::move(end)}};
    };

    auto f = [f = b.f](const auto &beg, const auto &end) {
        if (beg == end)
            return beg;
        decltype(auto) value = ltl::fast_invoke(f, *beg);
        return std::find_if_not(beg, end, [&](auto &x) { return ltl::fast_invoke(f, x) == value; });
    };

    auto advance = ltl::overloader{[f](increment_tag_t, const auto &beg, const auto &end) { return f(beg, end); },
                                   [f](decrement_tag_t, auto beg, const auto &end) {
                                       beg = safe_advance(beg, end, 1);
                                       return std::prev(f(beg, end));
                                   }};

    using Advance = decltype(advance);
    using Dereference = decltype(dereference);

    return Range{
        SplitIterator<it, Advance, Dereference, 0>{begin(FWD(a)), begin(FWD(a)), end(FWD(a)), advance, dereference},
        SplitIterator<it, Advance, Dereference, 0>{end(FWD(a)), begin(FWD(a)), end(FWD(a)), advance, dereference}};
}

} // namespace ltl
