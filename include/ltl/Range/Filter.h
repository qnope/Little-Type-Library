#pragma once

#include "Range.h"
#include "Reverse.h"
#include "../functional.h"

namespace ltl {
template <typename It, typename Predicate>
class FilterIterator :
    public BaseIterator<FilterIterator<It, Predicate>, It>,
    public WithSentinel<It>,
    public WithFunction<Predicate>,
    public IteratorOperationByIterating<FilterIterator<It, Predicate>>,
    public IteratorSimpleComparator<FilterIterator<It, Predicate>> {
  public:
    using reference = typename std::iterator_traits<It>::reference;
    DECLARE_EVERYTHING_BUT_REFERENCE(get_iterator_category<It>);

    FilterIterator() = default;

    FilterIterator(It it, It sentinelBegin, It sentinelEnd, Predicate function) :
        BaseIterator<FilterIterator, It>{std::move(it)},                    //
        WithSentinel<It>{std::move(sentinelBegin), std::move(sentinelEnd)}, //
        WithFunction<Predicate>{std::move(function)} {
        this->m_it = std::find_if(this->m_it, this->m_sentinelEnd, this->m_function);
    }

    FilterIterator &operator++() noexcept {
        this->m_it = std::find_if(std::next(this->m_it), this->m_sentinelEnd, this->m_function);
        return *this;
    }

    FilterIterator &operator--() noexcept {
        this->m_it = std::find_if(std::next(this->reverse(this->m_it)), this->m_sentinelBegin, this->m_function).m_it;
        return *this;
    }
};

template <typename F>
struct FilterType {
    F f;
};

template <typename... Fs>
constexpr auto filter(Fs... fs) {
    auto foo = compose(std::move(fs)...);
    return FilterType<decltype(foo)>{std::move(foo)};
}

template <typename F, typename... Fs, requires_f(!IsIterable<F>)>
constexpr auto remove_if(F f, Fs... fs) {
    auto foo = not_(compose(std::move(f), std::move(fs)...));
    return FilterType<decltype(foo)>{std::move(foo)};
}

template <typename F>
struct is_chainable_operation<FilterType<F>> : true_t {};

template <typename T1, typename F, requires_f(IsIterableRef<T1>)>
constexpr decltype(auto) operator|(T1 &&a, FilterType<F> b) {
    using std::begin;
    using std::end;
    using it = decltype(begin(FWD(a)));
    return Range{FilterIterator<it, decltype(b.f)>{begin(FWD(a)), begin(FWD(a)), end(FWD(a)), b.f},
                 FilterIterator<it, decltype(b.f)>{end(FWD(a)), begin(FWD(a)), end(FWD(a)), b.f}};
}
} // namespace ltl
