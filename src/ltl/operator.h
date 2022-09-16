/**
 * @file operator.h
 */
#pragma once

#include "concept.h"

#include <algorithm>
#include <deque>
#include <list>
#include <vector>

#include "Range/Filter.h"
#include "Range/Join.h"
#include "Range/Map.h"
#include "Range/Range.h"
#include "Range/Taker.h"
#include "condition.h"
#include "optional_type.h"

#define _(args, expr) [](LPL_MAP(LPL_ADD_AUTO, args)) { return expr; }

namespace ltl {

// To vector, deque, list
struct to_vector_t {};
struct to_deque_t {};
struct to_list_t {};
constexpr to_vector_t to_vector{};
constexpr to_deque_t to_deque{};
constexpr to_list_t to_list{};

using std::begin;
using std::end;

template <typename T1, requires_f(IsIterable<T1>)>
constexpr decltype(auto) operator|(T1 &&a, to_vector_t) {
    using value = typename std::iterator_traits<decltype(begin(FWD(a)))>::value_type;
    return std::vector<value>(begin(FWD(a)), end(FWD(a)));
}

template <typename T1, requires_f(IsIterable<T1>)>
constexpr decltype(auto) operator|(T1 &&a, to_deque_t) {
    using value = typename std::iterator_traits<decltype(begin(FWD(a)))>::value_type;
    return std::deque<value>(begin(FWD(a)), end(FWD(a)));
}

template <typename T1, requires_f(IsIterable<T1>)>
constexpr decltype(auto) operator|(T1 &&a, to_list_t) {
    using value = typename std::iterator_traits<decltype(begin(FWD(a)))>::value_type;
    return std::list<value>(begin(FWD(a)), end(FWD(a)));
}

} // namespace ltl
