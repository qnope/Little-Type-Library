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

  decltype(auto) operator[](std::size_t idx) const {
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

template <typename It> auto begin(const Range<It> &r) { return r.begin(); }

template <typename It> auto end(const Range<It> &r) { return r.end(); }

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
    void operator()(F *x) { x->~F(); }
  };

  NullableFunction() = default;
  NullableFunction(F f) : m_ptr{new (&m_memory) F{std::move(f)}} {}
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

  template <typename Args> decltype(auto) operator()(Args &&args) const {
    assert(m_ptr);
    return (*m_ptr)(FWD(args));
  }

  std::aligned_storage_t<sizeof(F)> m_memory;
  std::unique_ptr<F, DestructorPtr> m_ptr;
};

#define DECLARE_EVERYTHING_BUT_REFERENCE                                       \
  using pointer = AsPointer<reference>;                                        \
  using value_type = std::decay_t<reference>;                                  \
  using difference_type = std::size_t;                                         \
  using iterator_category = std::random_access_iterator_tag;

constexpr auto hasAdvanceUntilNext =
    IS_VALID((x), x.advanceUntilNext(IncrementTag{}),
             x.advanceUntilNext(DecrementTag{}));

#define DEFINE_BASE_ITERATOR(name, It, Function)                               \
public:                                                                        \
  DECLARE_EVERYTHING_BUT_REFERENCE                                             \
  COMPARABLE()                                                                 \
                                                                               \
  bool operator==(const name &it) const noexcept { return m_it == it.m_it; }   \
                                                                               \
  name &operator+=(long long int n) noexcept {                                 \
    if (n > 0) {                                                               \
      while (n--)                                                              \
        ++(*this);                                                             \
    }                                                                          \
                                                                               \
    else {                                                                     \
      while (n++)                                                              \
        --(*this);                                                             \
    }                                                                          \
    return *this;                                                              \
  }                                                                            \
                                                                               \
  name &operator-=(long long int n) noexcept {                                 \
    if (n > 0) {                                                               \
      while (n--)                                                              \
        --(*this);                                                             \
    }                                                                          \
                                                                               \
    else {                                                                     \
      while (n++)                                                              \
        ++(*this);                                                             \
    }                                                                          \
    return *this;                                                              \
  }                                                                            \
                                                                               \
  friend name operator+(name a, long long int n) noexcept {                    \
    a += n;                                                                    \
    return a;                                                                  \
  }                                                                            \
                                                                               \
  friend name operator+(long long int n, name a) noexcept {                    \
    a += n;                                                                    \
    return a;                                                                  \
  }                                                                            \
                                                                               \
  friend name operator-(name a, long long int n) noexcept {                    \
    a -= n;                                                                    \
    return a;                                                                  \
  }                                                                            \
                                                                               \
  friend name operator-(long long int n, name a) noexcept {                    \
    a -= n;                                                                    \
    return a;                                                                  \
  }                                                                            \
                                                                               \
  friend std::size_t operator-(const name &b, name a) noexcept {               \
    std::size_t d{0};                                                          \
    while (a != b) {                                                           \
      ++d;                                                                     \
      ++a;                                                                     \
    }                                                                          \
    return d;                                                                  \
  }                                                                            \
                                                                               \
private:                                                                       \
  It m_it{};                                                                   \
  It m_sentinelBegin{};                                                        \
  It m_sentinelEnd{};                                                          \
  NullableFunction<Function> m_function{};

#define DEFINE_CONSTRUCTOR_ITERATOR(name, It, Function)                        \
public:                                                                        \
  name() noexcept = default;                                                   \
                                                                               \
  name(It it) noexcept : m_it{it}, m_sentinelBegin{it}, m_sentinelEnd{it} {}   \
                                                                               \
  name(It it, It sentinelBegin, It sentinelEnd, Function function) noexcept    \
      : m_it{std::move(it)}, m_sentinelBegin{std::move(sentinelBegin)},        \
        m_sentinelEnd{std::move(sentinelEnd)},                                 \
        m_function{std::move(function)} {                                      \
    advanceUntilNext(IncrementTag{});                                          \
  }

#define DEFINE_DEREFERENCE_ITERATOR()                                          \
public:                                                                        \
  decltype(auto) operator*() const noexcept {                                  \
    assert(m_it != m_sentinelEnd);                                             \
    return *m_it;                                                              \
  }                                                                            \
                                                                               \
  auto operator->() const noexcept {                                           \
    return AsPointer<decltype(**this)>{**this};                                \
  }

#define DEFINE_INCREMENTATION_ITERATOR(name)                                   \
public:                                                                        \
  name &operator++() noexcept {                                                \
    assert(m_it != m_sentinelEnd);                                             \
    ++m_it;                                                                    \
    advanceUntilNext(IncrementTag{});                                          \
    return *this;                                                              \
  }                                                                            \
                                                                               \
  name &operator--() noexcept {                                                \
    assert(m_it != m_sentinelBegin);                                           \
    --m_it;                                                                    \
    advanceUntilNext(DecrementTag{});                                          \
    return *this;                                                              \
  }

template <typename It, typename Predicate> class FilterIterator {
public:
  using reference = typename std::iterator_traits<It>::reference;

  DEFINE_BASE_ITERATOR(FilterIterator, It, Predicate)
  DEFINE_CONSTRUCTOR_ITERATOR(FilterIterator, It, Predicate)
  DEFINE_DEREFERENCE_ITERATOR()
  DEFINE_INCREMENTATION_ITERATOR(FilterIterator)

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

template <typename It> class TakerIterator {
public:
  using reference = typename std::iterator_traits<It>::reference;

  DEFINE_BASE_ITERATOR(TakerIterator, It, Nothing)
  DEFINE_DEREFERENCE_ITERATOR()
  DEFINE_INCREMENTATION_ITERATOR(TakerIterator)

public:
  TakerIterator() = default;

  TakerIterator(It begin, It sentinelBegin, It sentinelEnd, std::size_t n)
      : m_it{std::move(begin)}, m_sentinelBegin{std::move(sentinelBegin)},
        m_sentinelEnd{std::move(sentinelEnd)}, m_n{n} {
    advanceUntilNext(IncrementTag{});
  }

  TakerIterator(It begin) : m_it{std::move(begin)} {}

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
}; // namespace ltl

template <typename It, typename Function> struct MapIterator {
  using reference =
      std::invoke_result_t<Function,
                           typename std::iterator_traits<It>::reference>;
  DEFINE_BASE_ITERATOR(MapIterator, It, Function)
  DEFINE_CONSTRUCTOR_ITERATOR(MapIterator, It, Function)
  DEFINE_INCREMENTATION_ITERATOR(MapIterator)

public:
  reference operator*() const { return m_function(*m_it); }
  pointer operator->() const { return m_function(*m_it); }
  void advanceUntilNext(...) {}
};

template <typename ValueType> struct ValueIterator {
  using reference = ValueType;

  DEFINE_BASE_ITERATOR(ValueIterator, ValueType, Nothing)

public:
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

  ValueType operator*() const noexcept { return this->m_it; }

  ValueType m_step;
};

template <typename... Iterators> struct ZipIterator {
  using reference =
      tuple_t<typename std::iterator_traits<Iterators>::reference...>;

  DEFINE_BASE_ITERATOR(ZipIterator, tuple_t<Iterators...>, Nothing)
  DEFINE_CONSTRUCTOR_ITERATOR(ZipIterator, tuple_t<Iterators...>, Nothing)

public:
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

  reference operator*() const {
    return TO_VARIADIC(this->m_it, xs, return reference{*xs...});
  }
  void advanceUntilNext(...) {}
};

#undef DECLARE_EVERYTHING_BUT_REFERENCE

template <typename... Containers> auto zip(Containers &&... containers) {
  // constexpr auto types = type_list_v<Containers...>;
  // typed_static_assert(!types.isEmpty);
  // because clang is shit...
  // typed_static_assert_msg(all_of_type(types, is_iterable),
  //                        "Zip operations must be used with containers");

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
  std::size_t n;
};

template <typename F> FilterType<std::decay_t<F>> filter(F &&f) {
  return {FWD(f)};
}

template <typename F> MapType<std::decay_t<F>> map(F &&f) { return {FWD(f)}; }

inline auto take_n(std::size_t n) { return TakerType{n}; }

LTL_MAKE_IS_KIND(FilterType, is_filter_type, IsFilterType, typename);
LTL_MAKE_IS_KIND(MapType, is_map_type, IsMapType, typename);

template <typename T>
constexpr auto IsUsefulForSmartIterator = IsFilterType<T> || IsMapType<T> ||
                                          (type_v<T> == type_v<TakerType>);

#define OP(Type, Iterator)                                                     \
  template <typename R, typename F, requires_f(IsIterable<R>)>                 \
  auto operator|(R &&r, Type<F> f) {                                           \
    auto begin = std::begin(FWD(r));                                           \
    auto end = std::end(FWD(r));                                               \
    using it = decltype(begin);                                                \
    return Range{Iterator<it, F>{begin, begin, end, std::move(f.f)},           \
                 Iterator<it, F>{end}};                                        \
  }

OP(FilterType, FilterIterator)
OP(MapType, MapIterator)
#undef OP

template <typename R, requires_f(IsIterable<R>)>
auto operator|(R &&r, TakerType taker) {
  auto begin = std::begin(FWD(r));
  auto end = std::end(FWD(r));
  return Range{TakerIterator<decltype(begin)>{begin, begin, end, taker.n},
               TakerIterator<decltype(begin)>{end}};
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

// because clang is shit
template <typename R> auto piper(R r) { return r; }

template <typename R, typename T, typename... Ts>
auto piper(R &&r, T &&t, Ts &&... ts) {
  return piper(FWD(r) | FWD(t), FWD(ts)...);
}

template <typename R, typename Tuple,
          requires_f(IsIterable<R> &&IsTuple<Tuple>)>
auto operator|(R &&r, Tuple &&chainFunctions) {
  return chainFunctions(
      [&r](auto &&... xs) { return piper(FWD(r), FWD(xs)...); });
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
