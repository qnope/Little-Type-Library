#pragma once

#include <algorithm>
#include <numeric>
#include <optional>

#include "../lpl/if.h"
#include "smart_iterator.h"
#include "tuple/template.h"
#include "type_t.h"
#include "type_traits.h"

namespace ltl {
#define ALGO_MONO_ITERATOR(name)                                               \
  template <typename C, typename... As, LTL_REQUIRE_T(is_iterable(type_v<C>))> \
  auto name(C &&c, As &&... as) {                                              \
    typed_static_assert(is_iterable(c));                                       \
    return std::name(std::begin(c), std::end(c), std::forward<As>(as)...);     \
  }

#define ALGO_DOUBLE_ITERATOR(name)                                             \
  template <typename C1, typename C2, typename... As,                          \
            LTL_REQUIRE_T(is_iterable(type_v<C1>) && is_iterable(type_v<C2>))> \
  auto name(C1 &&c1, C2 &&c2, As &&... as) {                                   \
    return std::name(std::begin(c1), std::end(c1), std::begin(c2),             \
                     std::end(c2), std::forward<As>(as)...);                   \
  }

// Version for finds
#define ALGO_FIND_VALUE(name) template<typename C, typename ...As, LTL_REQUIRE_T(is_iterable(type_v<C>))> \
    auto name(C &&c, As &&...as) -> std::optional<decltype(c.begin())> {\
        auto it = std::name(std::begin(c), std::end(c), std::forward<As>(as)...); \
        if(it == std::end(c)) \
            return std::nullopt; \
        return it; \
    }

#define ALGO_FIND_RANGE(name) template<typename C1, typename C2, typename ...As, LTL_REQUIRE_T(is_iterable(type_v<C1>) && is_iterable(type_v<C2>))> \
    auto name(C1 &&c1, C2 &&c2, As &&...as) -> std::optional<decltype(c1.begin())> {\
        auto it = std::name(std::begin(c1), std::end(c1), std::begin(c2), std::end(c2), std::forward<As>(as)...); \
        if(it == std::end(c1)) \
            return std::nullopt; \
        return it; \
    }

// Version for finds
LPL_MAP(ALGO_FIND_VALUE, find, find_if, find_if_not, adjacent_find)
LPL_MAP(ALGO_FIND_RANGE, find_first_of, find_end, search)

// Non modifying
LPL_MAP(ALGO_MONO_ITERATOR, all_of, any_of, none_of, count, count_if, for_each, mismatch)


// Modifying
LPL_MAP(ALGO_MONO_ITERATOR, copy, copy_if, copy_backward, move, move_backward,
        fill, transform, generate)

LPL_MAP(ALGO_MONO_ITERATOR, remove, remove_if, remove_copy, remove_copy_if,
        replace, replace_if, replace_copy, replace_copy_if)

LPL_MAP(ALGO_MONO_ITERATOR, swap_ranges, reverse, reverse_copy, shuffle, sample,
        unique, unique_copy)

// Partitioning
LPL_MAP(ALGO_MONO_ITERATOR, is_partitioned, partition, partition_copy,
        stable_partition, partition_point)

// Sorting
LPL_MAP(ALGO_MONO_ITERATOR, is_sorted, is_sorted_until)

template <typename C, typename... P,
          LTL_REQUIRE_T(is_iterable(type_v<C>) &&
                        !is_const(remove_reference(type_v<C>)))>
C sort(C &&c, P &&... p) {
  std::sort(std::begin(c), std::end(c), std::forward<P>(p)...);
  return c;
}

template <typename C, typename... P,
          LTL_REQUIRE_T(is_iterable(type_v<C>) &&
                        !is_const(remove_reference(type_v<C>)))>
C stable_sort(C &&c, P &&... p) {
  std::stable_sort(std::begin(c), std::end(c), std::forward<P>(p)...);
  return c;
}

template <typename C, LTL_REQUIRE_T(is_iterable(type_v<C>) &&
                                    is_const(remove_reference(type_v<C>)))>
[[nodiscard]] std::decay_t<C> sort(C &&c) {
  std::decay_t<C> newContainer;
  ltl::copy(c, ltl::sorted_inserter(newContainer));
  return newContainer;
}

// binary search operations
LPL_MAP(ALGO_MONO_ITERATOR, lower_bound, upper_bound, binary_search,
        equal_range)

// Other operations
ALGO_DOUBLE_ITERATOR(merge)
ALGO_MONO_ITERATOR(inplace_merge)

// set operations
LPL_MAP(ALGO_DOUBLE_ITERATOR, includes, set_difference, set_intersection,
        set_symmetric_difference, set_union)

// heap operation
LPL_MAP(ALGO_MONO_ITERATOR, is_heap, is_heap_until, make_heap, push_heap,
        pop_heap, sort_heap)

// max / min
LPL_MAP(ALGO_MONO_ITERATOR, max_element, min_element, minmax_element)

// comparison
LPL_MAP(ALGO_DOUBLE_ITERATOR, equal, lexicographical_compare)

// permutation
ALGO_DOUBLE_ITERATOR(is_permutation)
LPL_MAP(ALGO_MONO_ITERATOR, next_permutation, prev_permutation)

// numeric operations
LPL_MAP(ALGO_MONO_ITERATOR, iota, accumulate, inner_product,
        adjacent_difference, partial_sum)

#undef ALGO_MONO_ITERATOR
#undef ALGO_DOUBLE_ITERATOR
} // namespace ltl
