/**
 * @file Taker.h
 */
#pragma once

#include "ltl/ltl.h"
#include "Range.h"
#include "BaseIterator.h"

namespace ltl {

/**
 * \defgroup Iterator The iterator group
 * @{
 */

/// \cond

using std::begin;
using std::end;

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

template <typename F>
struct is_chainable_operation<TakeWhileType<F>> : true_t {};

template <typename F>
struct is_chainable_operation<DropWhileType<F>> : true_t {};

template <>
struct is_chainable_operation<DropNType> : true_t {};

template <>
struct is_chainable_operation<TakeNType> : true_t {};

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
                                            return ltl::fast_invoke(std::move(b.f), FWD(x));
                                        });
    return Range{begin(FWD(a)), sentinelEnd};
}

template <typename T1, typename F, requires_f(IsIterableRef<T1>)>
constexpr decltype(auto) operator|(T1 &&a, DropWhileType<F> b) {
    auto sentinelBegin = std::find_if_not(begin(FWD(a)), end(FWD(a)), [b = std::move(b)](auto &&x) { //
        return ltl::fast_invoke(std::move(b.f), FWD(x));
    });
    return Range{sentinelBegin, end(FWD(a))};
}

/// \endcond

/**
 * @brief take_n - take the first n elements
 *
 * @code
 *  std::vector<int> elements;
 *
 *  auto takenElements = elements | ltl::take_n(5);
 * @endcode
 * @param n
 */
inline auto take_n(std::size_t n) { return TakeNType{n}; }

/**
 * @brief drop_n - Remove the first n elements
 *
 * @code
 *  std::vector<int> elements;
 *
 *  auto takenElements = elements | ltl::drop_n(5);
 * @endcode
 * @param n
 */
inline auto drop_n(std::size_t n) { return DropNType{n}; }

template <typename F>
/**
 * @brief take_while - take while a predicate is true
 *
 * @code
 *  std::vector<int> sortedElements;
 *
 *  // all elements are less than 10
 *  auto takenElements = elements | ltl::take_while(ltl::less_than(10));
 * @endcode
 * @param f
 */
auto take_while(F f) {
    return TakeWhileType<F>{std::move(f)};
}

template <typename F>
/**
 * @brief drop_while - Drop while a predicate is true
 *
 * @code
 *  std::vector<int> sortedElements;
 *
 *  // all elements are greater or equal to 10
 *  auto takenElements = elements | ltl::drop_while(ltl::less_than(10)); *
 * @endcode
 *
 * @param f
 */
auto drop_while(F f) {
    return DropWhileType<F>{std::move(f)};
}

/// @}

} // namespace ltl
