#pragma once

#include "concept.h"

namespace ltl {
#define ALGO_MONO_ITERATOR(name)                                               \
  template <typename C, typename... As> auto name(C &&c, As &&... as) {        \
    typed_static_assert_msg(is_iterable(FWD(c)), "C must be iterable");        \
    return std::name(std::begin(FWD(c)), std::end(FWD(c)), FWD(as)...);        \
  }

#define ALGO_DOUBLE_ITERATOR(name)                                             \
  template <typename C1, typename C2, typename... As>                          \
  auto name(C1 &&c1, C2 &&c2, As &&... as) {                                   \
    typed_static_assert_msg(is_iterable(FWD(c1)) && is_iterable(FWD(c2)),      \
                            "C1 and C2 must be iterable");                     \
    return std::name(std::begin(FWD(c1)), std::end(FWD(c1)),                   \
                     std::begin(FWD(c2)), std::end(FWD(c2)), FWD(as)...);      \
  }

// Version for finds
#define ALGO_FIND_VALUE(name)                                                  \
  template <typename C, typename... As> auto name(C &&c, As &&... as) {        \
    typed_static_assert_msg(is_iterable(FWD(c)), "C must be iterable");        \
    auto it = std::name(std::begin(FWD(c)), std::end(FWD(c)), FWD(as)...);     \
    if (it == std::end(FWD(c)))                                                \
      return decltype(std::make_optional(it)){};                               \
    return std::make_optional(it);                                             \
  }                                                                            \
  template <typename C, typename... As>                                        \
  auto LPL_CAT(name, _value)(C && c, As && ... as) {                           \
    auto opt = name(FWD(c), FWD(as)...);                                       \
    if (!opt)                                                                  \
      return decltype(std::make_optional(**opt)){};                            \
    return std::make_optional(**opt);                                          \
  }                                                                            \
  template <typename C, typename... As>                                        \
  auto LPL_CAT(name, _ptr)(C && c, As && ... as) {                             \
    auto opt = name(FWD(c), FWD(as)...);                                       \
    if (!opt)                                                                  \
      return decltype(std::addressof(**opt)){};                                \
    return std::addressof(**opt);                                              \
  }

#define ALGO_FIND_RANGE(name)                                                  \
  template <typename C1, typename C2, typename... As>                          \
  auto name(C1 &&c1, C2 &&c2, As &&... as) {                                   \
    typed_static_assert_msg(is_iterable(FWD(c1)) && is_iterable(FWD(c2)),      \
                            "C1 and C2 must be iterable");                     \
    auto it = std::name(std::begin(FWD(c1)), std::end(FWD(c1)),                \
                        std::begin(FWD(c2)), std::end(FWD(c2)), FWD(as)...);   \
    if (it == std::end(FWD(c1)))                                               \
      return decltype(std::make_optional(it)){};                               \
    return std::make_optional(it);                                             \
  }

// Version for finds
LPL_MAP(ALGO_FIND_VALUE, find, find_if, find_if_not, adjacent_find)
LPL_MAP(ALGO_FIND_RANGE, find_first_of, find_end, search)

// Non modifying
LPL_MAP(ALGO_MONO_ITERATOR, all_of, any_of, none_of, count, count_if, for_each,
        mismatch)

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

// Sorting
template <typename C, typename... P> C sort(C &&c, P &&... p) {
  typed_static_assert_msg(is_iterable(FWD(c)) && !is_const(FWD(c)),
                          "C must not be const and must be iterable");
  std::sort(std::begin(c), std::end(c), FWD(p)...);
  return FWD(c);
}

template <typename C, typename... P> C stable_sort(C &&c, P &&... p) {
  typed_static_assert_msg(is_iterable(FWD(c)) && !is_const(FWD(c)),
                          "C must not be const and must be iterable");
  std::stable_sort(std::begin(c), std::end(c), FWD(p)...);
  return FWD(c);
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
#undef ALGO_FIND_RANGE
#undef ALGO_FIND_VALUE

template <typename C, typename Function, requires_f(IsIterable<C>)>
auto computeMean(const C &c, Function &&function)
    -> std::optional<decltype(*std::begin(c) / c.size())> {
  const auto size = c.size();
  if (size == 0)
    return std::nullopt;
  const auto total = accumulate(c, FWD(function));
  return total / c.size();
}
} // namespace ltl
