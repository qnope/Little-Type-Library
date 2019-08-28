#pragma once

#include "Tuple.h"
#include "concept.h"
#include "ltl.h"
#include "operator.h"

#include <deque>
#include <limits>
#include <list>
#include <memory>
#include <vector>

namespace ltl {

template <typename It> class Range {
public:
  template <typename R>
  Range(R &r) noexcept : m_it{std::begin(r)}, m_end{std::end(r)} {}

  Range(It it, It end) noexcept : m_it{std::move(it)}, m_end{std::move(end)} {}

  bool empty() const noexcept { return m_it == m_end; }

  std::size_t size() const noexcept { return std::distance(m_it, m_end); }

  auto begin() const { return m_it; }
  auto end() const { return m_end; }

  decltype(auto) operator[](std::size_t idx) {
    assert(idx < size());
    return *(m_it + idx);
  }

  decltype(auto) front() const noexcept {
    assert(!empty());
    return *m_it;
  }

  decltype(auto) back() const noexcept {
    assert(!empty());
    return *(m_it + std::size_t{size() - 1});
  }

private:
  It m_it;
  It m_end;
};

template <typename R> Range(R &r)->Range<decltype(std::begin(r))>;

LTL_MAKE_IS_KIND(Range, is_range, IsRange, typename);

template <typename T> struct AsPointer {
  AsPointer(T &&v) noexcept : v{std::move(v)} {}
  T &operator*() noexcept { return v; }
  T *operator->() noexcept { return std::addressof(v); }
  T v;
};

template <typename T> struct AsPointer<T &> {
  AsPointer(T &v) noexcept : v{std::addressof(v)} {}
  T &operator*() noexcept { return *v; }
  T *operator->() noexcept { return v; }
  T *v;
};

struct IncrementTag {};
struct DecrementTag {};
struct Nothing {};

template <typename F> struct NullableFunction {
  struct DestructorPtr {
    template <typename T> void operator()(T *x) { x->~T(); }
  };

  NullableFunction() = default;
  NullableFunction(F &&f) : m_ptr{new (&m_memory) F{std::move(f)}} {}
  NullableFunction(const NullableFunction &f) {
    if (f.m_ptr)
      m_ptr.reset(new (&m_memory) F{std::move(*f.m_ptr)});
  }

  NullableFunction &operator=(NullableFunction f) {
    m_ptr = nullptr;
    if (f.m_ptr)
      m_ptr.reset(new (&m_memory) F{std::move(*f.m_ptr)});
    return *this;
  }

  template <typename... Args> decltype(auto) operator()(Args &&... args) {
    assert(m_ptr);
    return (*m_ptr)(FWD(args)...);
  }

  std::aligned_storage_t<sizeof(F), alignof(F)> m_memory;
  std::unique_ptr<F, DestructorPtr> m_ptr;
};

template <typename DerivedIt, typename It, typename Function>
class BaseIterator : public crtp<DerivedIt, BaseIterator> {
  static constexpr auto hasAdvanceUntilNext =
      IS_VALID((x), x.advanceUntilNext(IncrementTag{}),
               x.advanceUntilNext(DecrementTag{}));

public:
  BaseIterator() = default;

  BaseIterator(It it) : m_it{it}, m_sentinelBegin{it}, m_sentinelEnd{it} {}

  BaseIterator(It &&it, It &&sentinelBegin, It &&sentinelEnd,
               Function &&function) noexcept
      : m_it{std::move(it)}, m_sentinelBegin{std::move(sentinelBegin)},
        m_sentinelEnd{std::move(sentinelEnd)}, m_function{std::move(function)} {
    DerivedIt &derived = this->underlying();
    if_constexpr(hasAdvanceUntilNext(derived)) {
      derived.advanceUntilNext(IncrementTag{});
    }
  }

  bool operator==(const DerivedIt &it) const noexcept {
    return this->underlying().m_it == it.m_it;
  }

  bool operator!=(const DerivedIt &it) const noexcept {
    return this->underlying().m_it != it.m_it;
  }

  DerivedIt &operator++() noexcept {
    DerivedIt &it = this->underlying();
    assert(it.m_it != it.m_sentinelEnd);
    ++it.m_it;
    if_constexpr(hasAdvanceUntilNext(it)) {
      it.advanceUntilNext(IncrementTag{});
    }
    return it;
  }

  DerivedIt &operator--() noexcept {
    DerivedIt &it = this->underlying();
    assert(it.m_it != it.m_sentinelBegin);
    --it.m_it;
    if_constexpr(hasAdvanceUntilNext(it)) {
      it.advanceUntilNext(DecrementTag{});
    }
    return it;
  }

  decltype(auto) operator*() noexcept {
    DerivedIt &it = this->underlying();
    assert(it.m_it != it.m_sentinelEnd);
    return *it.m_it;
  }

  auto operator-> () noexcept {
    DerivedIt &it = this->underlying();
    return AsPointer<decltype(*it)>{*it};
  }

  DerivedIt &operator+=(long long int n) noexcept {
    DerivedIt &it = this->underlying();
    if (n > 0) {
      while (n--)
        ++it;
    }

    else {
      while (n++)
        --it;
    }
    return it;
  }

  DerivedIt &operator-=(long long int n) noexcept {
    DerivedIt &it = this->underlying();
    if (n > 0) {
      while (n--)
        --it;
    }

    else {
      while (n++)
        ++it;
    }
    return it;
  }

  friend DerivedIt operator+(DerivedIt a, long long int n) noexcept {
    a += n;
    return a;
  }

  friend DerivedIt operator+(long long int n, DerivedIt a) noexcept {
    a += n;
    return a;
  }

  friend DerivedIt operator-(DerivedIt a, long long int n) noexcept {
    a -= n;
    return a;
  }

  friend DerivedIt operator-(long long int n, DerivedIt a) noexcept {
    a -= n;
    return a;
  }

  friend std::size_t operator-(const DerivedIt &b, DerivedIt a) noexcept {
    std::size_t d{0};
    while (a != b) {
      ++d;
      ++a;
    }
    return d;
  }

protected:
  It m_it{};
  It m_sentinelBegin{};
  It m_sentinelEnd{};
  NullableFunction<Function> m_function{};
};

#define DECLARE_EVERYTHING_BUT_REFERENCE                                       \
  using pointer = AsPointer<reference>;                                        \
  using value_type = std::decay_t<reference>;                                  \
  using difference_type = std::size_t;                                         \
  using iterator_category = std::random_access_iterator_tag;

template <typename It, typename Predicate>
class FilterIterator
    : public BaseIterator<FilterIterator<It, Predicate>, It, Predicate> {
  friend BaseIterator<FilterIterator, It, Predicate>;

public:
  using reference = typename std::iterator_traits<It>::reference;
  DECLARE_EVERYTHING_BUT_REFERENCE

  using BaseIterator<FilterIterator<It, Predicate>, It,
                     Predicate>::BaseIterator;

private:
  template <typename Tag> void advanceUntilNext(Tag) noexcept {
    if_constexpr(type_v<Tag> == type_v<IncrementTag>) {
      while (this->m_it != this->m_sentinelEnd &&
             !this->m_function(*this->m_it)) {
        ++this->m_it;
      }
    }

    else {
      while (this->m_it != this->m_sentinelBegin &&
             !this->m_function(*this->m_it)) {
        --this->m_it;
      }
      assert(this->m_function(*this->m_it));
    }
  }
};

template <typename It>
class TakerIterator : public BaseIterator<TakerIterator<It>, It, Nothing> {
  friend BaseIterator<TakerIterator, It, Nothing>;

public:
  using reference = typename std::iterator_traits<It>::reference;
  DECLARE_EVERYTHING_BUT_REFERENCE

  TakerIterator() = default;

  TakerIterator(It &&begin, It &&sentinelBegin, It &&sentinelEnd, std::size_t n)
      : m_n{n} {
    this->m_it = std::move(begin);
    this->m_sentinelBegin = std::move(sentinelBegin);
    this->m_sentinelEnd = std::move(sentinelEnd);
    advanceUntilNext(IncrementTag{});
  }

  TakerIterator(It &&begin)
      : BaseIterator<TakerIterator<It>, It, Nothing>{std::move(begin)} {}

private:
  template <typename Tag> void advanceUntilNext(Tag) noexcept {
    if_constexpr(type_v<Tag> == type_v<IncrementTag>) {
      if (m_n == 0) {
        this->m_it = this->m_sentinelEnd;
      } else
        --m_n;
    }

    else {
      ++m_n;
    }
  }

  std::size_t m_n{0};
};

template <typename It, typename Function>
struct MapIterator : BaseIterator<MapIterator<It, Function>, It, Function> {
  using reference =
      std::invoke_result_t<Function,
                           typename std::iterator_traits<It>::reference>;
  DECLARE_EVERYTHING_BUT_REFERENCE

  using BaseIterator<MapIterator<It, Function>, It, Function>::BaseIterator;

  reference operator*() { return this->m_function(*this->m_it); }
  pointer operator->() { return this->m_function(*this->m_it); }
};

template <typename ValueType>
struct ValueIterator
    : BaseIterator<ValueIterator<ValueType>, ValueType, Nothing> {
  using reference = ValueType;
  DECLARE_EVERYTHING_BUT_REFERENCE

  ValueIterator() noexcept {
    this->m_sentinelBegin = std::numeric_limits<ValueType>::lowest();
    this->m_sentinelEnd = std::numeric_limits<ValueType>::max();
    this->m_it = this->m_sentinelBegin;
    m_step = 1;
  }

  ValueIterator(ValueType value, ValueType step = 1) noexcept {
    this->m_sentinelBegin = std::numeric_limits<ValueType>::lowest();
    this->m_sentinelEnd = std::numeric_limits<ValueType>::max();
    this->m_it = value;
    m_step = step;
  }

  ValueIterator &operator++() noexcept {
    assert(this->m_it != this->m_sentinelEnd);
    this->m_it += m_step;
    return *this;
  }

  ValueIterator &operator--() noexcept {
    assert(this->m_it != this->m_sentinelBegin);
    this->m_it -= m_step;
    return *this;
  }

  ValueType operator*() noexcept { return this->m_it; }

  ValueType m_step;
};

template <typename... Iterators>
struct ZipIterator
    : BaseIterator<ZipIterator<Iterators...>, tuple_t<Iterators...>, Nothing> {
  using reference =
      tuple_t<typename std::iterator_traits<Iterators>::reference...>;

  using BaseIterator<ZipIterator<Iterators...>, tuple_t<Iterators...>,
                     Nothing>::BaseIterator;

  DECLARE_EVERYTHING_BUT_REFERENCE

  ZipIterator &operator++() {
    assert(this->m_it != this->m_sentinelEnd);
    TO_VARIADIC(this->m_it, xs, (++xs, ...));
    return *this;
  }

  ZipIterator &operator--() {
    assert(this->m_it != this->m_sentinelBegin);
    TO_VARIADIC(this->m_it, xs, (--xs, ...));
    return *this;
  }

  reference operator*() {
    return TO_VARIADIC(this->m_it, xs, return reference{*xs...});
  }
};

#undef DECLARE_EVERYTHING_BUT_REFERENCE

template <typename... Containers> auto zip(Containers &&... containers) {
  static_assert(sizeof...(Containers) > 0);
  typed_static_assert_msg((true_v && ... && is_iterable(containers)),
                          "Zip operations must be used with containers");

  assert(FROM_VARIADIC(FWD(containers))([](auto &&c1, auto &&... cs) {
    return (true && ... && (FWD(c1).size() == FWD(cs).size()));
  }));

  return Range{ZipIterator<decltype(std::begin(FWD(containers)))...>{
                   tuple_t{std::begin(FWD(containers))...},
                   tuple_t{std::begin(FWD(containers))...},
                   tuple_t{std::end(FWD(containers))...}, Nothing{}},
               ZipIterator<decltype(std::begin(FWD(containers)))...>{
                   tuple_t{std::end(FWD(containers))...}}};
}

template <typename ValueType> auto valueRange() {
  return Range{ValueIterator{std::numeric_limits<ValueType>::lowest()},
               ValueIterator{std::numeric_limits<ValueType>::max()}};
}

template <typename ValueType> auto valueRange(ValueType start) {
  return Range{ValueIterator{start},
               ValueIterator{std::numeric_limits<ValueType>::max()}};
}

template <typename ValueType> auto valueRange(ValueType start, ValueType end) {
  return Range{ValueIterator{start}, ValueIterator{end}};
}

template <typename ValueType> auto steppedValueRange(ValueType step) {
  auto begin = ValueIterator{std::numeric_limits<ValueType>::lowest(), step};
  auto end = ValueIterator{std::numeric_limits<ValueType>::max(), step};
  return Range{begin, end};
}

template <typename ValueType>
auto steppedValueRange(ValueType start, ValueType step) {
  auto begin = ValueIterator{start, step};
  auto end = ValueIterator{std::numeric_limits<ValueType>::max(), step};
  return Range{begin, end};
}

template <typename ValueType>
auto steppedValueRange(ValueType start, ValueType end, ValueType step) {
  auto begin = ValueIterator{start, step};
  auto _end = ValueIterator{end, step};
  return Range{begin, _end};
}

template <typename Container> auto enumerate(Container &&container) {
  return zip(valueRange<std::size_t>(0, container.size()), FWD(container));
}

LTL_MAKE_IS_KIND(FilterIterator, is_filter_iterator, IsFilterIterator,
                 typename);
LTL_MAKE_IS_KIND(MapIterator, is_map_iterator, IsMapIterator, typename);
LTL_MAKE_IS_KIND(TakerIterator, is_taker_iterator, IsTakerIterator, typename);
template <typename T>
constexpr auto IsSmartIterator =
    IsFilterIterator<T> || IsMapIterator<T> || IsTakerIterator<T>;

// Helper Types
template <typename F> struct FilterType { F f; };
template <typename F> struct MapType { F f; };
struct TakerType {
  std::size_t f;
};

template <typename F> auto filter(F &&f) {
  return FilterType<std::decay_t<F>>{FWD(f)};
}

template <typename F> auto map(F &&f) {
  return MapType<std::decay_t<F>>{FWD(f)};
}

auto take_n(std::size_t n) { return TakerType{n}; }

LTL_MAKE_IS_KIND(FilterType, is_filter_type, IsFilterType, typename);
LTL_MAKE_IS_KIND(MapType, is_map_type, IsMapType, typename);

template <typename T>
constexpr auto IsUsefulForSmartIterator = IsFilterType<T> || IsMapType<T> ||
                                          (type_v<T> == type_v<TakerType>);

// Pipe operator
namespace detail {
template <typename R, typename F> constexpr auto get_iterator_type() {
  using it = decltype(std::begin(std::declval<R>()));
  using f = decltype(std::declval<F>().f);
  if constexpr (IsFilterType<F>)
    return type_v<FilterIterator<it, f>>;
  else if constexpr (IsMapType<F>)
    return type_v<MapIterator<it, f>>;
  else if constexpr (type_v<F> == type_v<TakerType>)
    return type_v<TakerIterator<it>>;
  else
    compile_time_error(
        "You must use a valid type that is useful for smart iterators", F);
}

template <typename R, typename F>
using iterator_type = decltype_t((get_iterator_type<R, F>()));
} // namespace detail

template <typename R, typename F,
          requires_f(IsIterable<R> &&IsUsefulForSmartIterator<F>)>
auto operator|(R &&r, F f) {
  if_constexpr(!is_range(FWD(r))) {
    typed_static_assert_msg(
        !is_rvalue_reference(FWD(r)),
        "An owning container cannot be passed through rvalue_reference");
  }
  using it = detail::iterator_type<R, F>;
  return Range{it{std::begin(FWD(r)), std::begin(FWD(r)), std::end(FWD(r)),
                  std::move(f.f)},
               it{std::end(FWD(r))}};
}

// Chaining functions
template <
    typename F1, typename F2,
    requires_f(IsUsefulForSmartIterator<F1> &&IsUsefulForSmartIterator<F2>)>
auto operator|(F1 &&f1, F2 &&f2) {
  return tuple_t{FWD(f1), FWD(f2)};
}

template <typename Tuple, typename F,
          requires_f(IsTuple<Tuple> &&IsUsefulForSmartIterator<F>)>
auto operator|(Tuple &&tuple, F &&f) {
  return FWD(tuple).push_back(FWD(f));
}

template <typename R, typename Tuple,
          requires_f(IsIterable<R> &&IsTuple<Tuple>)>
auto operator|(R &&r, Tuple &&chainFunctions) {
  return FWD(chainFunctions)(
      [&r](auto &&... xs) { return (FWD(r) | ... | (FWD(xs))); });
}
// To vector, deque, list
struct to_vector_t {};
struct to_deque_t {};
struct to_list_t {};
constexpr to_vector_t to_vector{};
constexpr to_deque_t to_deque{};
constexpr to_list_t to_list{};

template <typename R, requires_f(IsRange<R>)>
auto operator|(R &&r, to_vector_t) noexcept {
  return std::vector<
      typename std::iterator_traits<decltype(std::begin(FWD(r)))>::value_type>(
      std::begin(FWD(r)), std::end(FWD(r)));
}

template <typename R, requires_f(IsRange<R>)>
auto operator|(R &&r, to_deque_t) noexcept {
  return std::deque<
      typename std::iterator_traits<decltype(std::begin(FWD(r)))>::value_type>(
      std::begin(FWD(r)), std::end(FWD(r)));
}

template <typename R, requires_f(IsRange<R>)>
auto operator|(R &&r, to_list_t) noexcept {
  return std::list<
      typename std::iterator_traits<decltype(std::begin(FWD(r)))>::value_type>(
      std::begin(FWD(r)), std::end(FWD(r)));
}
} // namespace ltl
