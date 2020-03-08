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
#include "Tuple.h"
#include "optional_type.h"

#define _LAMBDA_WRITE_AUTO(x, y, ...)                                          \
  auto &&x LPL_WHEN(LPL_IS_NOT_PARENTHESES(y))(                                \
      LPL_CAT(, , ) LPL_DEFER_TWICE(_LAMBDA_WRITE_AUTO_I)()(y, __VA_ARGS__))

#define _LAMBDA_WRITE_AUTO_I() _LAMBDA_WRITE_AUTO
#define _(variables, expr)                                                     \
  [](LPL_EVAL(LPL_DEFER(_LAMBDA_WRITE_AUTO)(LPL_STRIP_PARENTHESES(variables),  \
                                            (), ()))) { return expr; }

namespace ltl {
constexpr auto is_chainable_operation = [](auto type) {
  return is_filter_type(type) || is_map_type(type) ||
         is_take_while_type(type) || is_drop_while_type(type) ||
         type == type_v<TakeNType> || type == type_v<DropNType>;
};
// To vector, deque, list
struct to_vector_t {};
struct to_deque_t {};
struct to_list_t {};
constexpr to_vector_t to_vector{};
constexpr to_deque_t to_deque{};
constexpr to_list_t to_list{};

// reverse iterator
struct reverse_t {};
constexpr reverse_t reversed;

template <typename It> auto safe_advance(It beg, It end, std::size_t n) {
  while (n-- && beg != end)
    ++beg;
  return beg;
}

template <typename T1, typename T2>
constexpr decltype(auto) operator|(T1 &&a, T2 &&b) {
  using std::begin;
  using std::end;
  [[maybe_unused]] constexpr auto t1 = decay_from(a);
  [[maybe_unused]] constexpr auto t2 = decay_from(b);

  if constexpr (is_iterable(t1)) {
    using value =
        typename std::iterator_traits<decltype(begin(FWD(a)))>::value_type;

    [[maybe_unused]] auto beginIt = begin(FWD(a));
    [[maybe_unused]] auto endIt = end(FWD(a));
    using it = decltype(beginIt);

    if constexpr (t2 == ltl::type_v<to_vector_t>) {
      return std::vector<value>(beginIt, endIt);
    }

    else if constexpr (t2 == ltl::type_v<to_deque_t>) {
      return std::deque<value>(beginIt, endIt);
    }

    else if constexpr (t2 == ltl::type_v<to_list_t>) {
      return std::list<value>(beginIt, endIt);
    }

    else if constexpr (is_filter_type(t2)) {
      return Range{FilterIterator<it, std::decay_t<decltype(FWD(b).f)>>{
                       beginIt, beginIt, endIt, FWD(b).f},
                   FilterIterator<it, std::decay_t<decltype(FWD(b).f)>>{endIt}};
    }

    else if constexpr (is_map_type(t2)) {
      return Range{MapIterator<it, std::decay_t<decltype(FWD(b).f)>>{
                       beginIt, beginIt, endIt, FWD(b).f},
                   MapIterator<it, std::decay_t<decltype(FWD(b).f)>>{endIt}};
    }

    else if constexpr (t2 == type_v<TakeNType>) {
      auto sentinelEnd = safe_advance(beginIt, endIt, b.n);
      return Range{beginIt, sentinelEnd};
    }

    else if constexpr (is_take_while_type(t2)) {
      auto sentinelEnd = std::find_if_not(
          beginIt, endIt, [&b](auto &&x) { return ltl::invoke(b.f, FWD(x)); });
      return Range{beginIt, sentinelEnd};
    }

    else if constexpr (t2 == type_v<DropNType>) {
      auto sentinelBegin = safe_advance(beginIt, endIt, b.n);
      return Range{sentinelBegin, endIt};
    }

    else if constexpr (is_drop_while_type(t2)) {
      auto sentinelBegin = std::find_if_not(
          beginIt, endIt, [&b](auto &&x) { return ltl::invoke(b.f, FWD(x)); });
      return Range{sentinelBegin, endIt};
    }

    else if constexpr (is_tuple_t(t2)) {
      return FWD(b)([&a](auto &&... xs) {
        return (std::forward<T1>(a) | ... | (FWD(xs)));
      });
    }

    else if constexpr (t2 == type_v<reverse_t>) {
      return Range{FWD(a).rbegin(), FWD(a).rend()};
    }

    else if constexpr (t2 == type_v<join_t>) {
      return Range{JoinIterator<it>{beginIt, beginIt, endIt},
                   JoinIterator<it>{endIt}};
    }

    else {
      compile_time_error(
          "If a is an iterable, you must provide a range provided type, or a "
          "tuple of range provided type",
          T2);
    }
  }

  else if constexpr (is_optional(t1)) {
    static_assert(is_map_type(t2),
                  "If a is an optional, you must provide map type");
    if (a)
      return std::make_optional(ltl::invoke(FWD(b).f, *FWD(a)));
    return decltype(std::make_optional(ltl::invoke(FWD(b).f, *FWD(a)))){};
  }

  else if constexpr (is_chainable_operation(t1)) {
    static_assert(is_chainable_operation(t2) || is_tuple_t(t2),
                  "If a is a range provided type, you must provide "
                  "another range provided type");
    if constexpr (is_chainable_operation(t2)) {
      return tuple_t{FWD(a), FWD(b)};
    } else if constexpr (is_tuple_t(t2)) {
      return FWD(b).push_front(FWD(a));
    }
  }

  else if constexpr (is_tuple_t(t1)) {
    static_assert(is_chainable_operation(t2) || is_tuple_t(t2),
                  "If a is a tuple, you must provide a range provided type");
    if constexpr (is_chainable_operation(t2)) {
      return FWD(a).push_back(FWD(b));
    } else if constexpr (is_tuple_t(t2)) {
      return FWD(a) + FWD(b);
    }

  }

  else if constexpr (is_optional_type(t1)) {
    static_assert(is_map_type(t2),
                  "If a is an optional_type, you must provide map type");
    if_constexpr(a.has_value) { //
      return ltl::optional_type{ltl::invoke(FWD(b).f, *a)};
    }
    else return ltl::nullopt_type;
  }

  else {
    compile_time_error("You must use iterable, optional or range provided type "
                       "for this operator |",
                       T1);
  }
} // namespace ltl

template <typename T1, typename T2>
constexpr decltype(auto) operator>>(T1 &&a, T2 &&b) {
  using std::begin;
  using std::end;
  [[maybe_unused]] constexpr auto t1 = decay_from(a);
  [[maybe_unused]] constexpr auto t2 = decay_from(b);

  if constexpr (is_iterable(t1)) {
    static_assert(is_map_type(t2),
                  "If a is an iterable, b must be a map type joinable");
    return FWD(a) | FWD(b) | join;
  }

  else if constexpr (is_optional_type(t1)) {
    static_assert(is_map_type(t2),
                  "If a is an optional_type, you must provide map type");
    if_constexpr(a.has_value) { //
      typed_static_assert_msg(
          is_optional_type(ltl::invoke(FWD(b).f, *a)),
          "With >> notation, function must return optional_type");
      return ltl::invoke(FWD(b).f, *a);
    }
    else {
      return ltl::nullopt_type;
    }
  }

  else if constexpr (is_optional(t1)) {
    static_assert(is_map_type(t2),
                  "If a is an optional, you must provide map type");
    typed_static_assert_msg(is_optional(ltl::invoke(FWD(b).f, *a)),
                            "With >> notation, function must return optional");

    if (a)
      return ltl::invoke(FWD(b).f, *FWD(a));
    return decltype(ltl::invoke(FWD(b).f, *FWD(a))){};
  }
}
} // namespace ltl
