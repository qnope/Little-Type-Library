#pragma once

#include "../ltl.h"
#include "Range.h"
#include "BaseIterator.h"

namespace ltl {
using std::begin;
using std::end;

template <typename It>
auto safe_advance(It beg, It end, std::size_t n) {
    while (n-- && beg != end)
        ++beg;
    return beg;
}

struct TakeNType {
    std::size_t n;
};

struct DropNType {
    std::size_t n;
};

template <typename F>
struct TakeWhileType {
    F f;
};

template <typename F>
struct DropWhileType {
    F f;
};

inline auto take_n(std::size_t n) { return TakeNType{n}; }
inline auto drop_n(std::size_t n) { return DropNType{n}; }

template <typename F>
auto take_while(F f) {
    return TakeWhileType<F>{std::move(f)};
}
template <typename F>
auto drop_while(F f) {
    return DropWhileType<F>{std::move(f)};
}

// reverse iterator
struct reverse_t {};
constexpr reverse_t reversed;

template <typename F>
struct is_chainable_operation<TakeWhileType<F>> : true_t {};

template <typename F>
struct is_chainable_operation<DropWhileType<F>> : true_t {};

template <>
struct is_chainable_operation<DropNType> : true_t {};

template <>
struct is_chainable_operation<TakeNType> : true_t {};

template <>
struct is_chainable_operation<reverse_t> : true_t {};

template <typename T1, requires_f(IsIterableRef<T1>)>
constexpr decltype(auto) operator|(T1 &&a, reverse_t) {
    return Range{FWD(a).rbegin(), FWD(a).rend()};
}

template <typename T1, requires_f(IsIterableRef<T1>)>
constexpr decltype(auto) operator|(T1 &&a, TakeNType b) {
    auto sentinelEnd = safe_advance(begin(FWD(a)), end(FWD(a)), b.n);
    return Range{begin(FWD(a)), sentinelEnd};
}

template <typename T1, requires_f(IsIterableRef<T1>)>
constexpr decltype(auto) operator|(T1 &&a, DropNType b) {
    auto sentinelBegin = safe_advance(begin(FWD(a)), end(FWD(a)), b.n);
    return Range{sentinelBegin, end(FWD(a))};
}

template <typename T1, typename F, requires_f(IsIterableRef<T1>)>
constexpr decltype(auto) operator|(T1 &&a, TakeWhileType<F> b) {
    auto sentinelEnd = std::find_if_not(begin(FWD(a)), end(FWD(a)),
                                        [b = std::move(b)](auto &&x) { //
                                            return ltl::invoke(std::move(b.f), FWD(x));
                                        });
    return Range{begin(FWD(a)), sentinelEnd};
}

template <typename T1, typename F, requires_f(IsIterableRef<T1>)>
constexpr decltype(auto) operator|(T1 &&a, DropWhileType<F> b) {
    auto sentinelBegin = std::find_if_not(begin(FWD(a)), end(FWD(a)), [b = std::move(b)](auto &&x) { //
        return ltl::invoke(std::move(b.f), FWD(x));
    });
    return Range{sentinelBegin, end(FWD(a))};
}
} // namespace ltl
