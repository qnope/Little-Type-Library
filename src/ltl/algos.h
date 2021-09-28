/**
 * @file algos.h
 */
#pragma once

#include <algorithm>
#include <numeric>
#include <functional>

#include "Tuple.h"
#include "traits.h"
#include "invoke.h"
#include "concept.h"
#include "optional.h"
#include "Range/Range.h"

#ifdef __cpp_lib_constexpr_algorithms
#define LTL_CONSTEXPR_ALGO constexpr
#else
#define LTL_CONSTEXPR_ALGO
#endif

namespace ltl {

/**
 * \defgroup Algorithm The Algorithm group
 * @{
 */

#define MAKE_CALLER(f) [&f](auto &&... xs) { return ltl::fast_invoke(static_cast<F &&>(f), FWD(xs)...); }

using std::begin;
using std::end;
using std::size;

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto consecutive_values(C &c, std::size_t n, F f) {
    std::size_t consecutiveValues = 0;

    auto it = begin(c);
    auto first = it;
    auto e = end(c);
    for (; it != e; ++it) {
        if (ltl::fast_invoke(f, *it)) {
            if (consecutiveValues == 0)
                first = it;
            ++consecutiveValues;
        } else {
            consecutiveValues = 0;
        }
        if (consecutiveValues == n)
            return first;
    }
    return e;
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO bool has_consecutive_values(const C &c, std::size_t n, F &&f) {
    return consecutive_values(c, n, FWD(f)) != end(c);
}

// Non modifying
template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto all_of(const C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::all_of(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto any_of(const C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::any_of(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto none_of(const C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::none_of(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C, typename F, requires_f(IsIterable<C>)>
LTL_CONSTEXPR_ALGO auto for_each(C &&c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::for_each(begin(FWD(c)), end(FWD(c)), MAKE_CALLER(f));
}

template <typename C, typename V>
LTL_CONSTEXPR_ALGO auto count(const C &c, const V &v) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::count(begin(c), end(c), v);
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto count_if(const C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::count_if(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C1, typename C2>
LTL_CONSTEXPR_ALGO auto mismatch(C1 &c1, C2 &c2) {
    static_assert(IsIterable<C1> && IsIterable<C2>, "C1 and C2 must be iterable");
    auto [it1, it2] = std::mismatch(begin(c1), end(c1), begin(c2), end(c2));

    if (it1 == end(c1) && it2 == end(c2))
        return decltype(ltl::make_optional(tuple_t{it1, it2})){};

    return ltl::make_optional(tuple_t{it1, it2});
}

template <typename C1, typename C2, typename F>
LTL_CONSTEXPR_ALGO auto mismatch(C1 &c1, C2 &c2, F &&f) {
    static_assert(IsIterable<C1> && IsIterable<C2>, "C1 and C2 must be iterable");
    auto [it1, it2] = std::mismatch(begin(c1), end(c1), begin(c2), end(c2), MAKE_CALLER(f));

    if (it1 == end(c1) && it2 == end(c2))
        return decltype(ltl::make_optional(tuple_t{it1, it2})){};

    return ltl::make_optional(tuple_t{it1, it2});
}

template <typename C, typename V>
LTL_CONSTEXPR_ALGO auto find(C &c, const V &v) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::find(begin(c), end(c), v);
}

template <typename C, typename V>
LTL_CONSTEXPR_ALGO auto find_ptr(C &c, const V &v) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::find(begin(c), end(c), v);
    if (it != end(c)) {
        return std::addressof(*it);
    }
    return decltype(std::addressof(*it)){nullptr};
}

template <typename C, typename V>
LTL_CONSTEXPR_ALGO auto find_value(const C &c, const V &v) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::find(begin(c), end(c), v);
    if (it != end(c)) {
        return ltl::make_optional(*it);
    }
    return decltype(ltl::make_optional(*it)){};
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto find_if(C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::find_if(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto find_if_ptr(C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::find_if(begin(c), end(c), MAKE_CALLER(f));
    if (it != end(c)) {
        return std::addressof(*it);
    }
    return decltype(std::addressof(*it)){nullptr};
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto find_if_value(const C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::find_if(begin(c), end(c), MAKE_CALLER(f));
    if (it != end(c)) {
        return ltl::make_optional(*it);
    }
    return decltype(ltl::make_optional(*it)){};
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto find_if_not(C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::find_if_not(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto find_if_not_ptr(C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::find_if_not(begin(c), end(c), MAKE_CALLER(f));
    if (it != end(c)) {
        return std::addressof(*it);
    }
    return decltype(std::addressof(*it)){nullptr};
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto find_if_not_value(const C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::find_if_not(begin(c), end(c), MAKE_CALLER(f));
    if (it != end(c)) {
        return ltl::make_optional(*it);
    }
    return decltype(ltl::make_optional(*it)){};
}

template <typename C, typename V>
LTL_CONSTEXPR_ALGO ltl::optional<std::size_t> index_of(const C &c, V &&v) {
    static_assert(IsIterable<C>, "C must be iterable");
    if (auto it = find(c, FWD(v)); it != end(c)) {
        return std::size_t(std::distance(begin(c), it));
    }
    return {};
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO ltl::optional<std::size_t> index_if(const C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    if (auto it = find_if(c, FWD(f)); it != end(c)) {
        return std::size_t(std::distance(begin(c), it));
    }
    return {};
}

template <typename C1, typename C2>
LTL_CONSTEXPR_ALGO auto find_end(C1 &c1, C2 &c2) {
    static_assert(IsIterable<C1> && IsIterable<C2>, "C1 and C2 must be iterable");
    return std::find_end(begin(c1), end(c1), begin(c2), end(c2));
}

template <typename C1, typename C2, typename F>
LTL_CONSTEXPR_ALGO auto find_end(C1 &c1, C2 &c2, F &&f) {
    static_assert(IsIterable<C1> && IsIterable<C2>, "C1 and C2 must be iterable");
    return std::find_end(begin(c1), end(c1), begin(c2), end(c2), MAKE_CALLER(f));
}

template <typename C1, typename C2>
LTL_CONSTEXPR_ALGO auto find_first_of(C1 &c1, const C2 &c2) {
    static_assert(IsIterable<C1> && IsIterable<C2>, "C1 and C2 must be iterable");
    return std::find_first_of(begin(c1), end(c1), begin(c2), end(c2));
}

template <typename C1, typename C2, typename F>
LTL_CONSTEXPR_ALGO auto find_first_of(const C1 &c1, const C2 &c2, F &&f) {
    static_assert(IsIterable<C1> && IsIterable<C2>, "C1 and C2 must be iterable");
    return std::find_first_of(begin(c1), end(c1), begin(c2), end(c2), MAKE_CALLER(f));
}

template <typename C>
LTL_CONSTEXPR_ALGO auto adjacent_find(C &c) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::adjacent_find(begin(c), end(c));
}

template <typename C>
LTL_CONSTEXPR_ALGO auto adjacent_find_ptr(C &c) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::adjacent_find(begin(c), end(c));
    if (it != end(c)) {
        return std::addressof(*it);
    }
    return decltype(std::addressof(*it)){nullptr};
}

template <typename C>
LTL_CONSTEXPR_ALGO auto adjacent_find_value(const C &c) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::adjacent_find(begin(c), end(c));
    if (it != end(c)) {
        return ltl::make_optional(*it);
    }
    return decltype(ltl::make_optional(*it)){};
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto adjacent_find(C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::adjacent_find(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto adjacent_find_ptr(C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::adjacent_find(begin(c), end(c), MAKE_CALLER(f));
    if (it != end(c)) {
        return std::addressof(*it);
    }
    return decltype(std::addressof(*it)){nullptr};
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto adjacent_find_value(const C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::adjacent_find(begin(c), end(c), MAKE_CALLER(f));
    if (it != end(c)) {
        return ltl::make_optional(*it);
    }
    return decltype(ltl::make_optional(*it)){};
}

template <typename C1, typename C2>
LTL_CONSTEXPR_ALGO auto search(C1 &c1, const C2 &c2) {
    typed_static_assert_msg(is_iterable(c1) && is_iterable(c2), "C1 and C2 must be iterable");
    return std::search(begin(c1), end(c1), begin(c2), end(c2));
}

template <typename C1, typename C2, typename F>
LTL_CONSTEXPR_ALGO auto search(C1 &c1, const C2 &c2, F &&f) {
    typed_static_assert_msg(is_iterable(c1) && is_iterable(c2), "C1 and C2 must be iterable");
    return std::search(begin(c1), end(c1), begin(c2), end(c2), MAKE_CALLER(f));
}

template <typename C, typename Size, typename V>
LTL_CONSTEXPR_ALGO auto search_n(C &c, Size count, const V &v) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::search_n(begin(c), end(c), count, v);
}

template <typename C, typename Size, typename V>
LTL_CONSTEXPR_ALGO auto search_n_ptr(C &c, Size count, const V &v) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::search_n(begin(c), end(c), count, v);
}

template <typename C, typename Size, typename V>
LTL_CONSTEXPR_ALGO auto search_n_value(const C &c, Size count, const V &v) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::search_n(begin(c), end(c), count, v);
    if (it != end(c)) {
        return ltl::make_optional(*it);
    }
    return decltype(ltl::make_optional(*it)){};
}

template <typename C, typename Size, typename V, typename F>
LTL_CONSTEXPR_ALGO auto search_n(C &c, Size count, const V &v, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::search_n(begin(c), end(c), count, v, MAKE_CALLER(f));
}

template <typename C, typename Size, typename V, typename F>
LTL_CONSTEXPR_ALGO auto search_n_ptr(C &c, Size count, const V &v, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::search_n(begin(c), end(c), count, v, MAKE_CALLER(f));
    if (it != end(c)) {
        return std::addressof(*it);
    }
    return decltype(std::addressof(*it)){nullptr};
}

template <typename C, typename Size, typename V, typename F>
LTL_CONSTEXPR_ALGO auto search_n_value(const C &c, Size count, const V &v, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");

    auto it = std::search_n(begin(c), end(c), count, v, MAKE_CALLER(f));
    if (it != end(c)) {
        return ltl::make_optional(*it);
    }
    return decltype(ltl::make_optional(*it)){};
}

template <typename C, typename It>
LTL_CONSTEXPR_ALGO auto copy(const C &c, It &&it) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::copy(begin(c), end(c), FWD(it));
}

template <typename C, typename It, typename F>
LTL_CONSTEXPR_ALGO auto copy_if(const C &c, It &&it, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::copy_if(begin(c), end(c), FWD(it), MAKE_CALLER(f));
}

template <typename C, typename It>
LTL_CONSTEXPR_ALGO auto copy_backward(const C &c, It &&it) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::copy_backward(begin(c), end(c), FWD(it));
}

template <typename C, typename It>
LTL_CONSTEXPR_ALGO auto move(const C &c, It &&it) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::move(begin(c), end(c), FWD(it));
}

template <typename C, typename It>
LTL_CONSTEXPR_ALGO auto move_backward(const C &c, It &&it) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::move_backward(begin(c), end(c), FWD(it));
}

template <typename C, typename V>
LTL_CONSTEXPR_ALGO void fill(C &c, const V &v) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    std::fill(begin(c), end(c), v);
}

template <typename C, typename It, typename F, requires_f(IsIterable<C>)>
LTL_CONSTEXPR_ALGO auto transform(const C &c, It &&it, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::transform(begin(c), end(c), FWD(it), MAKE_CALLER(f));
}

template <typename C, typename G>
LTL_CONSTEXPR_ALGO auto generate(C &c, G &&g) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    return std::generate(begin(c), end(c), FWD(g));
}

template <typename C, typename V>
LTL_CONSTEXPR_ALGO auto remove(C &c, const V &v) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    return std::remove(begin(c), end(c), v);
}

template <typename C, typename F, requires_f(IsIterable<C>)>
LTL_CONSTEXPR_ALGO auto remove_if(C &c, F &&f) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    return std::remove_if(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C, typename It, typename T>
LTL_CONSTEXPR_ALGO auto remove_copy(const C &c, It &&it, const T &v) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::remove_copy(begin(c), end(c), FWD(it), v);
}

template <typename C, typename It, typename F>
LTL_CONSTEXPR_ALGO auto remove_copy_if(const C &c, It &&it, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::remove_copy_if(begin(c), end(c), FWD(it), MAKE_CALLER(f));
}

template <typename C, typename V>
LTL_CONSTEXPR_ALGO auto replace(C &c, const V &o, const V &n) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    return std::replace(begin(c), end(c), o, n);
}

template <typename C, typename F, typename V>
LTL_CONSTEXPR_ALGO auto replace_if(C &c, F &&f, const V &n) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    return std::replace_if(begin(c), end(c), MAKE_CALLER(f), n);
}

template <typename C, typename It, typename V>
LTL_CONSTEXPR_ALGO auto replace_copy(const C &c, It &&it, const V &o, const V &v) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::replace_copy(begin(c), end(c), FWD(it), o, v);
}

template <typename C, typename It, typename F, typename V>
LTL_CONSTEXPR_ALGO auto replace_copy_if(const C &c, It &&it, F &&f, const V &n) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::replace_copy_if(begin(c), end(c), FWD(it), MAKE_CALLER(f), n);
}

template <typename C>
LTL_CONSTEXPR_ALGO auto reverse(C &c) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    std::reverse(begin(c), end(c));
}

template <typename C, typename It>
LTL_CONSTEXPR_ALGO auto reverse_copy(const C &c, It &&it) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::reverse_copy(begin(c), end(c), FWD(it));
}

template <typename C, typename It, typename Size, typename G>
LTL_CONSTEXPR_ALGO auto sample(const C &c, It &&it, Size size, G &&g) {
    static_assert(IsIterable<C>, "C must be iterable");

    return std::sample(begin(c), end(c), FWD(it), size, FWD(g));
}

template <typename C, typename G>
LTL_CONSTEXPR_ALGO auto shuffle(C &c, G &&g) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    return std::shuffle(begin(c), end(c), FWD(g));
}

template <typename C>
LTL_CONSTEXPR_ALGO auto unique(C &c) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    return std::unique(begin(c), end(c));
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto unique(C &c, F &&f) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    return std::unique(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C, typename It>
LTL_CONSTEXPR_ALGO auto unique_copy(const C &c, It &&it) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::unique_copy(begin(c), end(c), FWD(it));
}

template <typename C, typename It, typename F>
LTL_CONSTEXPR_ALGO auto unique_copy(const C &c, It &&it, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::unique_copy(begin(c), end(c), FWD(it), MAKE_CALLER(f));
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto is_partitioned(const C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::is_partitioned(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto partition(C &c, F &&f) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    return std::partition(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C, typename ItTrue, typename ItFalse, typename F>
LTL_CONSTEXPR_ALGO auto partition_copy(const C &c, ItTrue &&itTrue, ItFalse &&itFalse, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::partition_copy(begin(c), end(c), FWD(itTrue), FWD(itFalse), MAKE_CALLER(f));
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto stable_partition(C &c, F &&f) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    return std::stable_partition(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto partition_point(C &c, F &&f) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    return std::partition_point(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C>
LTL_CONSTEXPR_ALGO auto is_sorted(const C &c) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::is_sorted(begin(c), end(c));
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto is_sorted(const C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::is_sorted(begin(c), end(c), MAKE_CALLER(f));
}

namespace detail {
template <typename C>
LTL_CONSTEXPR_ALGO decltype(auto) copy_if_needed(C &&c) {
    if_constexpr(is_const(FWD(c)) && is_fixed_size_array(FWD(c))) {
        ltl::remove_cvref_t<C> newC;
        copy(FWD(c), begin(newC));
        return newC;
    }
    else_if_constexpr(is_const(FWD(c)) || is_rvalue_reference(FWD(c))) { //
        return static_cast<ltl::remove_cvref_t<C>>(FWD(c));
    }
    else {
        return FWD(c);
    }
}
} // namespace detail

template <typename C>
LTL_CONSTEXPR_ALGO decltype(auto) sort(C &&c) {
    static_assert(IsIterable<C>, "C must be iterable");
    decltype(auto) cToSort = detail::copy_if_needed(FWD(c));
    std::sort(begin(cToSort), end(cToSort));
    return cToSort;
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO decltype(auto) sort(C &&c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    decltype(auto) cToSort = detail::copy_if_needed(FWD(c));
    std::sort(begin(cToSort), end(cToSort), MAKE_CALLER(f));
    return cToSort;
}

template <typename C>
LTL_CONSTEXPR_ALGO decltype(auto) stable_sort(C &&c) {
    static_assert(IsIterable<C>, "C must be iterable");
    decltype(auto) cToSort = detail::copy_if_needed(FWD(c));
    std::stable_sort(begin(cToSort), end(cToSort));
    return cToSort;
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO decltype(auto) stable_sort(C &&c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    decltype(auto) cToSort = detail::copy_if_needed(FWD(c));
    std::stable_sort(begin(cToSort), end(cToSort), MAKE_CALLER(f));
    return cToSort;
}

template <typename C, typename It>
LTL_CONSTEXPR_ALGO auto nth_element(C &c, It &&nth) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    return std::nth_element(begin(c), FWD(nth), end(c));
}

template <typename C, typename It, typename F>
LTL_CONSTEXPR_ALGO auto nth_element(C &c, It &&nth, F &&f) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    return std::nth_element(begin(c), FWD(nth), end(c), MAKE_CALLER(f));
}

template <typename C>
LTL_CONSTEXPR_ALGO auto nth_element_n(C &c, std::size_t n) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    auto it = std::next(begin(c), n);
    std::nth_element(begin(c), it, end(c));
    return it;
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto nth_element_n(C &c, std::size_t n, F &&f) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    auto it = std::next(begin(c), n);
    std::nth_element(begin(c), it, end(c), MAKE_CALLER(f));
    return it;
}

template <typename C, typename V>
LTL_CONSTEXPR_ALGO auto lower_bound(C &c, const V &v) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::lower_bound(begin(c), end(c), v);
}

template <typename C, typename V, typename F>
LTL_CONSTEXPR_ALGO auto lower_bound(C &c, const V &v, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::lower_bound(begin(c), end(c), v, MAKE_CALLER(f));
}

template <typename C, typename V>
LTL_CONSTEXPR_ALGO auto lower_bound_ptr(C &c, const V &v) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::lower_bound(begin(c), end(c), v);
    if (it == end(c)) {
        return decltype(std::addressof(*it)){};
    }
    return std::addressof(*it);
}

template <typename C, typename V, typename F>
LTL_CONSTEXPR_ALGO auto lower_bound_ptr(C &c, const V &v, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::lower_bound(begin(c), end(c), v, MAKE_CALLER(f));
    if (it == end(c)) {
        return decltype(std::addressof(*it)){};
    }
    return std::addressof(*it);
}

template <typename C, typename V>
LTL_CONSTEXPR_ALGO auto lower_bound_value(const C &c, const V &v) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::lower_bound(begin(c), end(c), v);
    if (it == end(c)) {
        return decltype(ltl::make_optional(*it)){};
    }
    return ltl::make_optional(*it);
}

template <typename C, typename V, typename F>
LTL_CONSTEXPR_ALGO auto lower_bound_value(const C &c, const V &v, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::lower_bound(begin(c), end(c), v, MAKE_CALLER(f));
    if (it == end(c)) {
        return decltype(ltl::make_optional(*it)){};
    }
    return ltl::make_optional(*it);
}

template <typename C, typename V>
LTL_CONSTEXPR_ALGO auto upper_bound(C &c, const V &v) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::upper_bound(begin(c), end(c), v);
}

template <typename C, typename V, typename F>
LTL_CONSTEXPR_ALGO auto upper_bound(C &c, const V &v, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::upper_bound(begin(c), end(c), v, MAKE_CALLER(f));
}

template <typename C, typename V>
LTL_CONSTEXPR_ALGO auto upper_bound_ptr(C &c, const V &v) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::upper_bound(begin(c), end(c), v);
    if (it == end(c)) {
        return decltype(std::addressof(*it)){};
    }
    return std::addressof(*it);
}

template <typename C, typename V, typename F>
LTL_CONSTEXPR_ALGO auto upper_bound_ptr(C &c, const V &v, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::upper_bound(begin(c), end(c), v, MAKE_CALLER(f));
    if (it == end(c)) {
        return decltype(std::addressof(*it)){};
    }
    return std::addressof(*it);
}

template <typename C, typename V>
LTL_CONSTEXPR_ALGO auto upper_bound_value(const C &c, const V &v) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::upper_bound(begin(c), end(c), v);
    if (it == end(c)) {
        return decltype(ltl::make_optional(*it)){};
    }
    return ltl::make_optional(*it);
}

template <typename C, typename V, typename F>
LTL_CONSTEXPR_ALGO auto upper_bound_value(const C &c, const V &v, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::upper_bound(begin(c), end(c), v, MAKE_CALLER(f));
    if (it == end(c)) {
        return decltype(ltl::make_optional(*it)){};
    }
    return ltl::make_optional(*it);
}

template <typename C, typename V>
LTL_CONSTEXPR_ALGO auto binary_search(const C &c, const V &v) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::binary_search(begin(c), end(c), v);
}

template <typename C, typename V, typename F>
LTL_CONSTEXPR_ALGO auto binary_search(const C &c, const V &v, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::binary_search(begin(c), end(c), v, MAKE_CALLER(f));
}

template <typename C, typename V>
LTL_CONSTEXPR_ALGO auto equal_range(C &c, const V &v) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto [b, e] = std::equal_range(begin(c), end(c), v);
    return Range{b, e};
}

template <typename C, typename V, typename F>
LTL_CONSTEXPR_ALGO auto equal_range(C &c, const V &v, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto [b, e] = std::equal_range(begin(c), end(c), v, MAKE_CALLER(f));
    return Range{b, e};
}

template <typename C1, typename C2, typename It>
LTL_CONSTEXPR_ALGO auto merge(const C1 &c1, const C2 &c2, It &&it) {
    typed_static_assert_msg(is_iterable(c1) && is_iterable(c2), "C1 and C2 must be iterable");
    return std::merge(begin(c1), end(c1), begin(c2), end(c2), FWD(it));
}

template <typename C1, typename C2, typename It, typename F>
LTL_CONSTEXPR_ALGO auto merge(const C1 &c1, const C2 &c2, It &&it, F &&f) {
    typed_static_assert_msg(is_iterable(c1) && is_iterable(c2), "C1 and C2 must be iterable");
    return std::merge(begin(c1), end(c1), begin(c2), end(c2), FWD(it), MAKE_CALLER(f));
}

template <typename C, typename It>
LTL_CONSTEXPR_ALGO auto inplace_merge(C &c, It &&nth) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    return std::inplace_merge(begin(c), FWD(nth), end(c));
}

template <typename C, typename It, typename F>
LTL_CONSTEXPR_ALGO auto inplace_merge(C &c, It &&nth, F &&f) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    return std::inplace_merge(begin(c), FWD(nth), end(c), MAKE_CALLER(f));
}

template <typename C1, typename C2>
LTL_CONSTEXPR_ALGO auto includes(const C1 &c1, const C2 &c2) {
    typed_static_assert_msg(is_iterable(c1) && is_iterable(c2), "C1 and C2 must be iterable");
    return std::includes(begin(c1), end(c1), begin(c2), end(c2));
}

template <typename C1, typename C2, typename F>
LTL_CONSTEXPR_ALGO auto includes(const C1 &c1, const C2 &c2, F &&f) {
    typed_static_assert_msg(is_iterable(c1) && is_iterable(c2), "C1 and C2 must be iterable");
    return std::includes(begin(c1), end(c1), begin(c2), end(c2), MAKE_CALLER(f));
}

template <typename C1, typename C2, typename It>
LTL_CONSTEXPR_ALGO auto set_difference(const C1 &c1, const C2 &c2, It &&it) {
    typed_static_assert_msg(is_iterable(c1) && is_iterable(c2), "C1 and C2 must be iterable");
    return std::set_difference(begin(c1), end(c1), begin(c2), end(c2), FWD(it));
}

template <typename C1, typename C2, typename It, typename F>
LTL_CONSTEXPR_ALGO auto set_difference(const C1 &c1, const C2 &c2, It &&it, F &&f) {
    typed_static_assert_msg(is_iterable(c1) && is_iterable(c2), "C1 and C2 must be iterable");
    return std::set_difference(begin(c1), end(c1), begin(c2), end(c2), FWD(it), MAKE_CALLER(f));
}

template <typename C1, typename C2, typename It>
LTL_CONSTEXPR_ALGO auto set_intersection(const C1 &c1, const C2 &c2, It &&it) {
    typed_static_assert_msg(is_iterable(c1) && is_iterable(c2), "C1 and C2 must be iterable");
    return std::set_intersection(begin(c1), end(c1), begin(c2), end(c2), FWD(it));
}

template <typename C1, typename C2, typename It, typename F>
LTL_CONSTEXPR_ALGO auto set_intersection(const C1 &c1, const C2 &c2, It &&it, F &&f) {
    typed_static_assert_msg(is_iterable(c1) && is_iterable(c2), "C1 and C2 must be iterable");
    return std::set_intersection(begin(c1), end(c1), begin(c2), end(c2), FWD(it), MAKE_CALLER(f));
}

template <typename C1, typename C2, typename It>
LTL_CONSTEXPR_ALGO auto set_symmetric_difference(const C1 &c1, const C2 &c2, It &&it) {
    typed_static_assert_msg(is_iterable(c1) && is_iterable(c2), "C1 and C2 must be iterable");
    return std::set_symmetric_difference(begin(c1), end(c1), begin(c2), end(c2), FWD(it));
}

template <typename C1, typename C2, typename It, typename F>
LTL_CONSTEXPR_ALGO auto set_symmetric_difference(const C1 &c1, const C2 &c2, It &&it, F &&f) {
    typed_static_assert_msg(is_iterable(c1) && is_iterable(c2), "C1 and C2 must be iterable");
    return std::set_symmetric_difference(begin(c1), end(c1), begin(c2), end(c2), FWD(it), MAKE_CALLER(f));
}

template <typename C1, typename C2, typename It>
LTL_CONSTEXPR_ALGO auto set_union(const C1 &c1, const C2 &c2, It &&it) {
    typed_static_assert_msg(is_iterable(c1) && is_iterable(c2), "C1 and C2 must be iterable");
    return std::set_union(begin(c1), end(c1), begin(c2), end(c2), FWD(it));
}

template <typename C1, typename C2, typename It, typename F>
LTL_CONSTEXPR_ALGO auto set_union(const C1 &c1, const C2 &c2, It &&it, F &&f) {
    typed_static_assert_msg(is_iterable(c1) && is_iterable(c2), "C1 and C2 must be iterable");
    return std::set_union(begin(c1), end(c1), begin(c2), end(c2), FWD(it), MAKE_CALLER(f));
}

template <typename C>
LTL_CONSTEXPR_ALGO auto is_heap(const C &c) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::is_heap(begin(c), end(c));
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto is_heap(const C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::is_heap(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C>
LTL_CONSTEXPR_ALGO auto make_heap(C &c) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be an iterable and not const");
    return std::make_heap(begin(c), end(c));
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto make_heap(C &c, F &&f) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be an iterable and not const");
    return std::make_heap(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C>
LTL_CONSTEXPR_ALGO auto push_heap(C &c) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be an iterable and not const");
    return std::push_heap(begin(c), end(c));
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto push_heap(C &c, F &&f) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be an iterable and not const");
    return std::push_heap(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C>
LTL_CONSTEXPR_ALGO auto pop_heap(C &c) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be an iterable and not const");
    return std::pop_heap(begin(c), end(c));
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto pop_heap(C &c, F &&f) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be an iterable and not const");
    return std::pop_heap(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C>
LTL_CONSTEXPR_ALGO auto sort_heap(C &c) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be an iterable and not const");
    return std::sort_heap(begin(c), end(c));
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto sort_heap(C &c, F &&f) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be an iterable and not const");
    return std::sort_heap(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C>
LTL_CONSTEXPR_ALGO auto max_element(C &c) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::max_element(begin(c), end(c));
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto max_element(C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::max_element(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C>
LTL_CONSTEXPR_ALGO auto max_element_ptr(C &c) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::max_element(begin(c), end(c));
    if (it == end(c))
        return decltype(std::addressof(*it)){};
    return std::addressof(*it);
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto max_element_ptr(C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::max_element(begin(c), end(c), MAKE_CALLER(f));
    if (it == end(c))
        return decltype(std::addressof(*it)){};
    return std::addressof(*it);
}

template <typename C>
LTL_CONSTEXPR_ALGO auto max_element_value(const C &c) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::max_element(begin(c), end(c));
    if (it == end(c))
        return decltype(ltl::make_optional(*it)){};
    return ltl::make_optional(*it);
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto max_element_value(const C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::max_element(begin(c), end(c), MAKE_CALLER(f));
    if (it == end(c))
        return decltype(ltl::make_optional(*it)){};
    return ltl::make_optional(*it);
}

template <typename C>
LTL_CONSTEXPR_ALGO auto min_element(C &c) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::min_element(begin(c), end(c));
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto min_element(C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::min_element(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C>
LTL_CONSTEXPR_ALGO auto min_element_ptr(C &c) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::min_element(begin(c), end(c));
    if (it == end(c))
        return decltype(std::addressof(*it)){};
    return std::addressof(*it);
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto min_element_ptr(C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::min_element(begin(c), end(c), MAKE_CALLER(f));
    if (it == end(c))
        return decltype(std::addressof(*it)){};
    return std::addressof(*it);
}

template <typename C>
LTL_CONSTEXPR_ALGO auto min_element_value(const C &c) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::min_element(begin(c), end(c));
    if (it == end(c))
        return decltype(ltl::make_optional(*it)){};
    return ltl::make_optional(*it);
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto min_element_value(const C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto it = std::min_element(begin(c), end(c), MAKE_CALLER(f));
    if (it == end(c))
        return decltype(ltl::make_optional(*it)){};
    return ltl::make_optional(*it);
}

template <typename C>
LTL_CONSTEXPR_ALGO auto minmax_element(C &c) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::minmax_element(begin(c), end(c));
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto minmax_element(C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::minmax_element(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C>
LTL_CONSTEXPR_ALGO auto minmax_element_ptr(C &c) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto pair = std::minmax_element(begin(c), end(c));
    if (pair.first == end(c))
        return decltype(ltl::make_optional(tuple_t{std::addressof(*pair.first), std::addressof(*pair.second)})){};
    return ltl::make_optional(tuple_t{std::addressof(*pair.first), std::addressof(*pair.second)});
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto minmax_element_ptr(C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto pair = std::minmax_element(begin(c), end(c), MAKE_CALLER(f));
    if (pair.first == end(c))
        return decltype(ltl::make_optional(tuple_t{std::addressof(*pair.first), std::addressof(*pair.second)})){};
    return ltl::make_optional(tuple_t{std::addressof(*pair.first), std::addressof(*pair.second)});
}

template <typename C>
LTL_CONSTEXPR_ALGO auto minmax_element_value(const C &c) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto pair = std::minmax_element(begin(c), end(c));
    if (pair.first == end(c))
        return decltype(ltl::make_optional(tuple_t{*pair.first, *pair.second})){};
    return ltl::make_optional(tuple_t{*pair.first, *pair.second});
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto minmax_element_value(const C &c, F &&f) {
    typed_static_assert_msg(is_iterable(c), "C must be iterable");
    auto pair = std::minmax_element(begin(c), end(c), MAKE_CALLER(f));
    if (pair.first == end(c))
        return decltype(ltl::make_optional(tuple_t{*pair.first, *pair.second})){};
    return ltl::make_optional(tuple_t{*pair.first, *pair.second});
}

template <typename C1, typename C2>
LTL_CONSTEXPR_ALGO auto equal(const C1 &c1, const C2 &c2) {
    typed_static_assert_msg(is_iterable(c1) && is_iterable(c2), "C1 and C2 must be iterable");
    return std::equal(begin(c1), end(c1), begin(c2), end(c2));
}

template <typename C1, typename C2, typename F>
LTL_CONSTEXPR_ALGO auto equal(const C1 &c1, const C2 &c2, F &&f) {
    typed_static_assert_msg(is_iterable(c1) && is_iterable(c2), "C1 and C2 must be iterable");
    return std::equal(begin(c1), end(c1), begin(c2), end(c2), MAKE_CALLER(f));
}

template <typename C1, typename C2>
LTL_CONSTEXPR_ALGO auto lexicographical_compare(const C1 &c1, const C2 &c2) {
    typed_static_assert_msg(is_iterable(c1) && is_iterable(c2), "C1 and C2 must be iterable");
    return std::lexicographical_compare(begin(c1), end(c1), begin(c2), end(c2));
}

template <typename C1, typename C2, typename F>
LTL_CONSTEXPR_ALGO auto lexicographical_compare(const C1 &c1, const C2 &c2, F &&f) {
    typed_static_assert_msg(is_iterable(c1) && is_iterable(c2), "C1 and C2 must be iterable");
    return std::lexicographical_compare(begin(c1), end(c1), begin(c2), end(c2), MAKE_CALLER(f));
}

template <typename C1, typename C2>
LTL_CONSTEXPR_ALGO auto is_permutation(const C1 &c1, const C2 &c2) {
    typed_static_assert_msg(is_iterable(c1) && is_iterable(c2), "C1 and C2 must be iterable");
    return std::is_permutation(begin(c1), end(c1), begin(c2), end(c2));
}

template <typename C1, typename C2, typename F>
LTL_CONSTEXPR_ALGO auto is_permutation(const C1 &c1, const C2 &c2, F &&f) {
    typed_static_assert_msg(is_iterable(c1) && is_iterable(c2), "C1 and C2 must be iterable");
    return std::is_permutation(begin(c1), end(c1), begin(c2), end(c2), MAKE_CALLER(f));
}

template <typename C>
LTL_CONSTEXPR_ALGO auto next_permutation(C &c) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    return std::next_permutation(begin(c), end(c));
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto next_permutation(C &c, F &&f) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    return std::next_permutation(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C>
LTL_CONSTEXPR_ALGO auto prev_permutation(C &c) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    return std::prev_permutation(begin(c), end(c));
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto prev_permutation(C &c, F &&f) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    return std::prev_permutation(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C, typename V>
LTL_CONSTEXPR_ALGO auto iota(C &c, V &&v) {
    typed_static_assert_msg(is_iterable(c) && !is_const(c), "C must be iterable and not const");
    return std::iota(begin(c), end(c), FWD(v));
}

template <typename C, typename T>
constexpr T accumulate(const C &c, T init) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto b = begin(c);
    auto e = end(c);
    for (; b != e; ++b) {
        init = std::move(init) + *b;
    }

    return init;
}

template <typename C, typename T, typename BinaryOperation>
constexpr T accumulate(const C &c, T init, BinaryOperation &&op) {
    static_assert(IsIterable<C>, "C must be iterable");
    auto b = begin(c);
    auto e = end(c);
    for (; b != e; ++b) {
        init = ltl::invoke(FWD(op), std::move(init), *b);
    }
    return init;
}

template <typename C, typename It, typename T>
LTL_CONSTEXPR_ALGO auto inner_product(const C &c, It &&it, T &&init) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::inner_product(begin(c), end(c), FWD(it), FWD(init));
}

template <typename C, typename It, typename T, typename F1, typename F2>
LTL_CONSTEXPR_ALGO auto inner_product(const C &c, It &&it, T &&init, F1 &&f1, F2 &&f2) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::inner_product(
        begin(c), end(c), FWD(it), FWD(init),
        [&f1](auto &&... xs) { return ltl::fast_invoke(static_cast<F1 &&>(f1), FWD(xs)...); },
        [&f2](auto &&... xs) { return ltl::fast_invoke(static_cast<F2 &&>(f2), FWD(xs)...); });
}

template <typename C, typename It>
LTL_CONSTEXPR_ALGO auto adjacent_difference(const C &c, It &&it) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::adjacent_difference(begin(c), end(c), FWD(it));
}

template <typename C, typename It, typename F>
LTL_CONSTEXPR_ALGO auto adjacent_difference(const C &c, It &&it, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::adjacent_difference(begin(c), end(c), FWD(it), MAKE_CALLER(f));
}

template <typename C, typename It, requires_f(IsIterable<C>)>
LTL_CONSTEXPR_ALGO auto partial_sum(const C &c, It &&it) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::partial_sum(begin(c), end(c), FWD(it));
}

template <typename C, typename It, typename F, requires_f(IsIterable<C>)>
LTL_CONSTEXPR_ALGO auto partial_sum(const C &c, It &&it, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return std::partial_sum(begin(c), end(c), FWD(it), MAKE_CALLER(f));
}

template <typename C>
LTL_CONSTEXPR_ALGO auto computeMean(const C &c) {
    static_assert(IsIterable<C>, "C must be iterable");

    if (c.empty()) {
        return ltl::optional<decltype(std::accumulate(std::next(begin(c)), end(c), *begin(c)) / size(c))>{};
    }

    const auto total = std::accumulate(std::next(begin(c)), end(c), *begin(c));
    return ltl::make_optional(total / size(c));
}

template <typename C, typename V>
LTL_CONSTEXPR_ALGO auto contains(const C &c, V &&v) {
    static_assert(IsIterable<C>, "C must be iterable");

    return find(c, FWD(v)) != end(c);
}

template <typename C, typename F>
LTL_CONSTEXPR_ALGO auto contains_if(const C &c, F &&f) {
    static_assert(IsIterable<C>, "C must be iterable");
    return find_if(c, FWD(f)) != end(c);
}

template <typename C>
LTL_CONSTEXPR_ALGO auto map_contains(const C &c, const typename ltl::remove_cvref_t<C>::key_type &k) {
    return c.find(FWD(k)) != c.end();
}

template <typename C>
LTL_CONSTEXPR_ALGO auto map_find(C &&c, const typename ltl::remove_cvref_t<C>::key_type &k) {
    return FWD(c).find(FWD(k));
}

template <typename C>
LTL_CONSTEXPR_ALGO auto map_find_value(C &&c, const typename ltl::remove_cvref_t<C>::key_type &k) {
    auto it = FWD(c).find(FWD(k));
    if (it == FWD(c).end()) {
        return ltl::optional<decltype(it->second)>{};
    }
    return ltl::make_optional(it->second);
}

template <typename C>
LTL_CONSTEXPR_ALGO auto map_find_ptr(C &c, const typename ltl::remove_cvref_t<C>::key_type &k) {
    auto it = c.find(FWD(k));
    if (it == c.end()) {
        return decltype(std::addressof(it->second)){nullptr};
    }
    return std::addressof(it->second);
}

template <typename C>
LTL_CONSTEXPR_ALGO auto map_take(C &c, const typename ltl::remove_cvref_t<C>::key_type &k) {
    auto it = c.find(FWD(k));
    if (it == c.end()) {
        return ltl::optional<decltype(it->second)>{};
    }
    auto result = ltl::make_optional(std::move(it->second));
    c.erase(it);
    return result;
}

#undef MAKE_CALLER

/**
 * @}
 */

} // namespace ltl
