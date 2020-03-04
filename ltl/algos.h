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

template <typename C, typename It> auto copy(const C &c, It &&it) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return std::copy(begin(c), end(c), FWD(it));
}

template <typename C, typename It, typename F>
auto copy_if(const C &c, It &&it, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return std::copy_if(begin(c), end(c), FWD(it), MAKE_CALLER(f));
}

template <typename C, typename It> auto copy_backward(const C &c, It &&it) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return std::copy_backward(begin(c), end(c), FWD(it));
}

template <typename C, typename It> auto move(const C &c, It &&it) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return std::move(begin(c), end(c), FWD(it));
}

template <typename C, typename It> auto move_backward(const C &c, It &&it) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return std::move_backward(begin(c), end(c), FWD(it));
}

template <typename C, typename V> void fill(const C &c, const V &v) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  std::fill(begin(c), end(c), v);
}

template <typename C, typename It, typename F>
auto transform(const C &c, It &&it, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return std::transform(begin(c), end(c), FWD(it), MAKE_CALLER(f));
}

template <typename C, typename G> auto generate(const C &c, G &&g) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return std::generate(begin(c), end(c), FWD(g));
}

template <typename C, typename V> auto remove(C &c, const V &v) {
  typed_static_assert_msg(is_iterable(c) && !is_const(c),
                          "C must be iterable and not const");
  return std::remove(begin(c), end(c), v);
}

template <typename C, typename F> auto remove_if(C &c, F &&f) {
  typed_static_assert_msg(is_iterable(c) && !is_const(c),
                          "C must be iterable and not const");
  return std::remove_if(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C, typename It, typename T>
auto remove_copy(const C &c, It &&it, const T &v) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return std::remove_copy(begin(c), end(c), FWD(it), v);
}

template <typename C, typename It, typename F>
auto remove_copy_if(const C &c, It &&it, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return std::remove_copy_if(begin(c), end(c), FWD(it), MAKE_CALLER(f));
}

template <typename C, typename V> auto replace(C &c, const V &o, const V &n) {
  typed_static_assert_msg(is_iterable(c) && !is_const(c),
                          "C must be iterable and not const");
  return std::replace(begin(c), end(c), o, n);
}

template <typename C, typename F, typename V>
auto replace_if(C &c, F &&f, const V &n) {
  typed_static_assert_msg(is_iterable(c) && !is_const(c),
                          "C must be iterable and not const");
  return std::replace_if(begin(c), end(c), MAKE_CALLER(f), n);
}

template <typename C, typename It, typename V>
auto replace_copy(C &c, It &&it, const V &o, const V &v) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return std::replace_copy(begin(c), end(c), FWD(it), o, v);
}

template <typename C, typename It, typename F, typename V>
auto replace_copy_if(C &c, It &&it, F &&f, const V &n) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return std::replace_copy_if(begin(c), end(c), FWD(it), MAKE_CALLER(f), n);
}

template <typename C> auto reverse(C &c) {
  typed_static_assert_msg(is_iterable(c) && !is_const(c),
                          "C must be iterable and not const");
  std::reverse(begin(c), end(c));
}

template <typename C, typename It> auto reverse_copy(const C &c, It &&it) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return std::reverse_copy(begin(c), end(c), FWD(it));
}

template <typename C, typename It, typename Size, typename G>
auto sample(const C &c, It &&it, Size size, G &&g) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");

  return std::sample(begin(c), end(c), FWD(it), size, FWD(g));
}

template <typename C, typename G> auto shuffle(C &c, G &&g) {
  typed_static_assert_msg(is_iterable(c) && !is_const(c),
                          "C must be iterable and not const");
  return std::shuffle(begin(c), end(c), FWD(g));
}

template <typename C> auto unique(C &c) {
  typed_static_assert_msg(is_iterable(c) && !is_const(c),
                          "C must be iterable and not const");
  return std::unique(begin(c), end(c));
}

template <typename C, typename F> auto unique(C &c, F &&f) {
  typed_static_assert_msg(is_iterable(c) && !is_const(c),
                          "C must be iterable and not const");
  return std::unique(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C, typename It> auto unique_copy(const C &c, It &&it) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return std::unique_copy(begin(c), end(c), FWD(it));
}

template <typename C, typename It, typename F>
auto unique_copy(const C &c, It &&it, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return std::unique_copy(begin(c), end(c), FWD(it), MAKE_CALLER(f));
}

template <typename C, typename F> auto is_partitioned(const C &c, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return std::is_partitioned(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C, typename F> auto partition(C &c, F &&f) {
  typed_static_assert_msg(is_iterable(c) && !is_const(c),
                          "C must be iterable and not const");
  return std::partition(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C, typename ItTrue, typename ItFalse, typename F>
auto partition_copy(const C &c, ItTrue &&itTrue, ItFalse &&itFalse, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return std::partition_copy(begin(c), end(c), FWD(itTrue), FWD(itFalse),
                             MAKE_CALLER(f));
}

template <typename C, typename F> auto stable_partition(C &c, F &&f) {
  typed_static_assert_msg(is_iterable(c) && !is_const(c),
                          "C must be iterable and not const");
  return std::stable_partition(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C, typename F> auto partition_point(C &c, F &&f) {
  typed_static_assert_msg(is_iterable(c) && !is_const(c),
                          "C must be iterable and not const");
  return std::partition_point(begin(c), end(c), MAKE_CALLER(f));
}

template <typename C> auto is_sorted(const C &c) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return std::is_sorted(begin(c), end(c));
}

template <typename C, typename F> auto is_sorted(const C &c, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  return std::is_sorted(begin(c), end(c), MAKE_CALLER(f));
}

namespace detail {
template <typename C> decltype(auto) copy_if_needed(C &&c) {
  if_constexpr(is_const(FWD(c))) {
    std::decay_t<C> newC;
    constexpr auto isReservable = IS_VALID((x, size), x.reserve(size));
    if_constexpr(isReservable(newC, FWD(c).size())) {
      newC.reserve(FWD(c).size());
      copy(FWD(c), std::back_inserter(newC));
    }
    else {
      copy(FWD(c), begin(newC));
    }
    return newC;
  }
  else_if_constexpr(is_rvalue_reference(FWD(c))) {
    return static_cast<std::decay_t<C>>(FWD(c));
  }
  else {
    return FWD(c);
  }
}
} // namespace detail

template <typename C> decltype(auto) sort(C &&c) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  decltype(auto) cToSort = detail::copy_if_needed(FWD(c));
  std::sort(begin(cToSort), end(cToSort));
  return cToSort;
}

template <typename C, typename F> decltype(auto) sort(C &&c, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  decltype(auto) cToSort = detail::copy_if_needed(FWD(c));
  std::sort(begin(cToSort), end(cToSort), MAKE_CALLER(f));
  return cToSort;
}

template <typename C> decltype(auto) stable_sort(C &&c) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  decltype(auto) cToSort = detail::copy_if_needed(FWD(c));
  std::stable_sort(begin(cToSort), end(cToSort));
  return cToSort;
}

template <typename C, typename F> decltype(auto) stable_sort(C &&c, F &&f) {
  typed_static_assert_msg(is_iterable(c), "C must be iterable");
  decltype(auto) cToSort = detail::copy_if_needed(FWD(c));
  std::stable_sort(begin(cToSort), end(cToSort), MAKE_CALLER(f));
  return cToSort;
}

// Partitioning
LPL_MAP(ALGO_MONO_ITERATOR, is_partitioned, partition, partition_copy,
        stable_partition, partition_point)

// Sorting
LPL_MAP(ALGO_MONO_ITERATOR, is_sorted, is_sorted_until)

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
