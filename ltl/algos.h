#pragma once

#include <algorithm>
#include <numeric>
#include <optional>

#include "concept.h"
#include "traits.h"

namespace ltl {
#define ALGO_MONO_ITERATOR(name)                                               \
  template <typename C, typename... As> auto name(C &&c, As &&... as) {        \
    typed_static_assert_msg(is_iterable(FWD(c)), "C must be iterable");        \
    using std::begin;                                                          \
    using std::end;                                                            \
    return std::name(begin(FWD(c)), end(FWD(c)), FWD(as)...);                  \
  }

#define ALGO_DOUBLE_ITERATOR(name)                                             \
  template <typename C1, typename C2, typename... As>                          \
  auto name(C1 &&c1, C2 &&c2, As &&... as) {                                   \
    typed_static_assert_msg(is_iterable(FWD(c1)) && is_iterable(FWD(c2)),      \
                            "C1 and C2 must be iterable");                     \
    using std::begin;                                                          \
    using std::end;                                                            \
    return std::name(begin(FWD(c1)), end(FWD(c1)), begin(FWD(c2)),             \
                     end(FWD(c2)), FWD(as)...);                                \
  }

// Version for finds
#define ALGO_FIND_VALUE(name)                                                  \
  template <typename C, typename... As> auto name(C &&c, As &&... as) {        \
    typed_static_assert_msg(is_iterable(FWD(c)), "C must be iterable");        \
    using std::begin;                                                          \
    using std::end;                                                            \
    auto it = std::name(begin(FWD(c)), end(FWD(c)), FWD(as)...);               \
    if (it == end(FWD(c)))                                                     \
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
    using std::begin;                                                          \
    using std::end;                                                            \
    auto it = std::name(begin(FWD(c1)), end(FWD(c1)), begin(FWD(c2)),          \
                        end(FWD(c2)), FWD(as)...);                             \
    if (it == std::end(FWD(c1)))                                               \
      return decltype(std::make_optional(it)){};                               \
    return std::make_optional(it);                                             \
  }

#define MAKE_CALLER(f)                                                         \
  [&f](auto &&... xs) { return ltl::invoke(std::forward<F>(f), FWD(xs)...); }

using std::begin;
using std::end;

// Non modifying
template <typename C, typename F> auto all_of(const C &c, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return std::all_of(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C, typename F> auto any_of(const C &c, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return std::any_of(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C, typename F> auto none_of(const C &c, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return std::none_of(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C, typename F, requires_f(IsIterable<C>)>
auto for_each(C &&c, F &&f) {
  typed_static_assert_msg(is_iterable(FWD(c)), "C must be iterable");
  return std::for_each(begin(FWD(c)), end(FWD(c)), MAKE_CALLER(f));
}

template <typename It, typename Size, typename F>
auto for_each_n(It it, Size n, F &&f) {
  return std::for_each_n(it, n, MAKE_CALLER(f));
}

template <typename C, typename V> auto count(const C &c, const V &v) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return std::count(begin(c), end(c), v);
}

template <typename C, typename F> auto count_if(const C &c, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return std::count_if(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C1, typename C2> auto mismatch(C1 &c1, C2 &c2) {
  typed_static_assert_msg(is_iterable(c1) && is_iterable(c2),
                          "C1 and C2 must be iterable");
  auto it = std::mismatch(begin(c1), end(c1), begin(c2), end(c2));
  if (it.first == end(c1) || it.second == end(c2))
    return decltype(std::make_optional(it)){};
  return std::make_optional(it);
}

template <typename C1, typename C2, typename F>
auto mismatch(C1 &c1, C2 &c2, F &&f) {
  typed_static_assert_msg(is_iterable(c1) && is_iterable(c2),
                          "C1 and C2 must be iterable");
  auto it =
      std::mismatch(begin(c1), end(c1), begin(c2), end(c2), MAKE_CALLER(f));
  if (it.first == end(c1) || it.second == end(c2))
    return decltype(std::make_optional(it)){};
  return std::make_optional(it);
}

template <typename C, typename V> auto find(C &c, const V &v) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  auto it = std::find(begin(c), end(c), v);
  if (it != end(c)) {
    return std::make_optional(it);
  }
  return decltype(std::make_optional(it)){};
}

template <typename C, typename V> auto find_ptr(C &c, const V &v) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  auto it = std::find(begin(c), end(c), v);
  if (it != end(c)) {
    return std::addressof(*it);
  }
  return decltype(std::addressof(*it)){nullptr};
}

template <typename C, typename V> auto find_value(const C &c, const V &v) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  auto it = std::find(begin(c), end(c), v);
  if (it != end(c)) {
    return std::make_optional(*it);
  }
  return decltype(std::make_optional(*it)){};
}

template <typename C, typename F> auto find_if(C &c, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  auto it = std::find_if(begin(c), end(c), MAKE_CALLER(f));
  if (it != end(c)) {
    return std::make_optional(it);
  }
  return decltype(std::make_optional(it)){};
}

template <typename C, typename F> auto find_if_ptr(C &c, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  auto it = std::find_if(begin(c), end(c), MAKE_CALLER(f));
  if (it != end(c)) {
    return std::addressof(*it);
  }
  return decltype(std::addressof(*it)){nullptr};
}

template <typename C, typename F> auto find_if_value(const C &c, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  auto it = std::find_if(begin(c), end(c), MAKE_CALLER(f));
  if (it != end(c)) {
    return std::make_optional(*it);
  }
  return decltype(std::make_optional(*it)){};
}

template <typename C, typename F> auto find_if_not(C &c, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  auto it = std::find_if_not(begin(c), end(c), MAKE_CALLER(f));
  if (it != end(c)) {
    return std::make_optional(it);
  }
  return decltype(std::make_optional(it)){};
}

template <typename C, typename F> auto find_if_not_ptr(C &c, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  auto it = std::find_if_not(begin(c), end(c), MAKE_CALLER(f));
  if (it != end(c)) {
    return std::addressof(*it);
  }
  return decltype(std::addressof(*it)){nullptr};
}

template <typename C, typename F> auto find_if_not_value(const C &c, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  auto it = std::find_if_not(begin(c), end(c), MAKE_CALLER(f));
  if (it != end(c)) {
    return std::make_optional(*it);
  }
  return decltype(std::make_optional(*it)){};
}

template <typename C1, typename C2> auto find_end(C1 &c1, C2 &c2) {
  typed_static_assert_msg(is_iterable(c1) && is_iterable(c2),
                          "C1 and C2 must be iterable");
  auto it = std::find_end(begin(c1), end(c1), begin(c2), end(c2));
  if (it != end(c1))
    return std::make_optional(it);
  return decltype(std::make_optional(it)){};
}

template <typename C1, typename C2, typename F>
auto find_end(C1 &c1, C2 &c2, F &&f) {
  typed_static_assert_msg(is_iterable(c1) && is_iterable(c2),
                          "C1 and C2 must be iterable");
  auto it =
      std::find_end(begin(c1), end(c1), begin(c2), end(c2), MAKE_CALLER(f));
  if (it != end(c1))
    return std::make_optional(it);
  return decltype(std::make_optional(it)){};
}

template <typename C1, typename C2> auto find_first_of(C1 &c1, C2 &c2) {
  typed_static_assert_msg(is_iterable(c1) && is_iterable(c2),
                          "C1 and C2 must be iterable");
  auto it = std::find_first_of(begin(c1), end(c1), begin(c2), end(c2));
  if (it != end(c1))
    return std::make_optional(it);
  return decltype(std::make_optional(it)){};
}

template <typename C1, typename C2, typename F>
auto find_first_of(const C1 &c1, const C2 &c2, F &&f) {
  typed_static_assert_msg(is_iterable(c1) && is_iterable(c2),
                          "C1 and C2 must be iterable");
  auto it = std::find_first_of(begin(c1), end(c1), begin(c2), end(c2),
                               MAKE_CALLER(f));
  if (it != end(c1))
    return std::make_optional(it);
  return decltype(std::make_optional(it)){};
}

template <typename C> auto adjacent_find(C &c) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  auto it = std::adjacent_find(begin(c), end(c));
  if (it != end(c)) {
    return std::make_optional(it);
  }
  return decltype(std::make_optional(it)){};
}

template <typename C> auto adjacent_find_ptr(C &c) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  auto it = std::adjacent_find(begin(c), end(c));
  if (it != end(c)) {
    return std::addressof(*it);
  }
  return decltype(std::addressof(*it)){nullptr};
}

template <typename C> auto adjacent_find_value(const C &c) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  auto it = std::adjacent_find(begin(c), end(c));
  if (it != end(c)) {
    return std::make_optional(*it);
  }
  return decltype(std::make_optional(*it)){};
}

template <typename C, typename F> auto adjacent_find(C &c, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  auto it = std::adjacent_find(begin(c), end(c), MAKE_CALLER(f));
  if (it != end(c)) {
    return std::make_optional(it);
  }
  return decltype(std::make_optional(it)){};
}

template <typename C, typename F> auto adjacent_find_ptr(C &c, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  auto it = std::adjacent_find(begin(c), end(c), MAKE_CALLER(f));
  if (it != end(c)) {
    return std::addressof(*it);
  }
  return decltype(std::addressof(*it)){nullptr};
}

template <typename C, typename F> auto adjacent_find_value(const C &c, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  auto it = std::adjacent_find(begin(c), end(c), MAKE_CALLER(f));
  if (it != end(c)) {
    return std::make_optional(*it);
  }
  return decltype(std::make_optional(*it)){};
}

template <typename C1, typename C2> auto search(C1 &c1, C2 &c2) {
  typed_static_assert_msg(is_iterable(c1) && is_iterable(c2),
                          "C1 and C2 must be iterable");
  auto it = std::find_first_of(begin(c1), end(c1), begin(c2), end(c2));
  if (it != end(c1))
    return std::make_optional(it);
  return decltype(std::make_optional(it)){};
}

template <typename C1, typename C2, typename F>
auto search(C1 &c1, C2 &c2, F &&f) {
  typed_static_assert_msg(is_iterable(c1) && is_iterable(c2),
                          "C1 and C2 must be iterable");
  auto it = std::search(begin(c1), end(c1), begin(c2), end(c2), MAKE_CALLER(f));
  if (it != end(c1))
    return std::make_optional(it);
  return decltype(std::make_optional(it)){};
}

template <typename C, typename Size, typename V>
auto search_n(C &c, Size count, const V &v) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  auto it = std::search_n(begin(c), end(c), count, v);
  if (it != end(c)) {
    return std::make_optional(it);
  }
  return decltype(std::make_optional(it)){};
}

template <typename C, typename Size, typename V>
auto search_n_ptr(C &c, Size count, const V &v) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  auto it = std::search_n(begin(c), end(c), count, v);
  if (it != end(c)) {
    return std::addressof(*it);
  }
  return decltype(std::addressof(*it)){nullptr};
}

template <typename C, typename Size, typename V>
auto search_n_value(const C &c, Size count, const V &v) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  auto it = std::search_n(begin(c), end(c), count, v);
  if (it != end(c)) {
    return std::make_optional(*it);
  }
  return decltype(std::make_optional(*it)){};
}

template <typename C, typename Size, typename V, typename F>
auto search_n(C &c, Size count, const V &v, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  auto it = std::search_n(begin(c), end(c), count, v, MAKE_CALLER(f));
  if (it != end(c)) {
    return std::make_optional(it);
  }
  return decltype(std::make_optional(it)){};
}

template <typename C, typename Size, typename V, typename F>
auto search_n_ptr(C &c, Size count, const V &v, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  auto it = std::search_n(begin(c), end(c), count, v, MAKE_CALLER(f));
  if (it != end(c)) {
    return std::addressof(*it);
  }
  return decltype(std::addressof(*it)){nullptr};
}

template <typename C, typename Size, typename V, typename F>
auto search_n_value(const C &c, Size count, const V &v, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");

  auto it = std::search_n(begin(c), end(c), count, v, MAKE_CALLER(f));
  if (it != end(c)) {
    return std::make_optional(*it);
  }
  return decltype(std::make_optional(*it)){};
}

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
  using std::begin;
  using std::end;
  std::sort(begin(c), end(c), FWD(p)...);
  return FWD(c);
}

template <typename C, typename... P> C stable_sort(C &&c, P &&... p) {
  typed_static_assert_msg(is_iterable(FWD(c)) && !is_const(FWD(c)),
                          "C must not be const and must be iterable");
  using std::begin;
  using std::end;
  std::stable_sort(begin(c), end(c), FWD(p)...);
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
LPL_MAP(ALGO_MONO_ITERATOR, iota, inner_product, adjacent_difference,
        partial_sum)

#undef ALGO_MONO_ITERATOR
#undef ALGO_DOUBLE_ITERATOR
#undef ALGO_FIND_RANGE
#undef ALGO_FIND_VALUE

template <class C, class T> constexpr T accumulate(const C &c, T init) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  for (const auto &e : c) {
    init = std::move(init) + e;
  }
  return init;
}

template <class C, class T, class BinaryOperation>
constexpr T accumulate(const C &c, T init, BinaryOperation op) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  for (const auto &e : c) {
    init = op(std::move(init), e);
  }
  return init;
}

template <typename C> auto computeMean(const C &c) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  using std::begin;
  using std::end;
  using std::size;

  if (c.empty()) {
    return std::optional<decltype(
        std::accumulate(std::next(begin(c)), end(c), *begin(c)) / size(c))>{};
  }

  const auto total = std::accumulate(std::next(begin(c)), end(c), *begin(c));
  return std::make_optional(total / size(c));
}

template <typename C, typename V> auto contains(const C &c, V &&v) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");

  return static_cast<bool>(find(c, FWD(v)));
}

template <typename C, typename F> auto contains_if(const C &c, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return static_cast<bool>(find_if(c, FWD(f)));
}

template <typename C, typename K> auto contains_map(const C &c, K &&k) {
  return c.find(FWD(k)) != c.end();
}

template <typename C, typename K> auto find_map_value(C &&c, K &&k) {
  auto it = FWD(c).find(FWD(k));
  if (it == FWD(c).end()) {
    return std::optional<decltype(it->second)>{};
  }
  return std::make_optional(it->second);
}

template <typename C, typename K> auto find_map_ptr(C &c, K &&k) {
  auto it = c.find(FWD(k));
  if (it == c.end()) {
    return decltype(std::addressof(it->second)){nullptr};
  }
  return std::addressof(it->second);
}

template <typename C, typename K> auto take_map(C &c, K &&k) {
  auto it = c.find(FWD(k));
  if (it == c.end()) {
    return std::optional<decltype(it->second)>{};
  }
  auto result = std::make_optional(std::move(it->second));
  c.erase(it);
  return result;
}

template <typename C, typename... P>
auto min_element_value(const C &c, P &&... p) {
  using std::begin;
  using std::end;
  if (c.empty()) {
    return std::decay_t<decltype(*begin(c))>{};
  }
  return *min_element(c, FWD(p)...);
}

template <typename C, typename... P>
auto max_element_value(const C &c, P &&... p) {
  using std::begin;
  using std::end;
  if (c.empty()) {
    return std::decay_t<decltype(*begin(c))>{};
  }
  return *max_element(c, FWD(p)...);
}

template <typename C, typename... P>
auto minmax_element_value(const C &c, P &&... p) {
  using std::begin;
  using std::end;
  using underlying = std::decay_t<decltype(*begin(c))>;
  using T = std::pair<underlying, underlying>;
  if (c.empty()) {
    return T{};
  }
  auto [min, max] = minmax_element(c, FWD(p)...);
  return T{*min, *max};
}

} // namespace ltl
