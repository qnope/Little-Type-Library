#pragma once

#include "ltl.h"

namespace ltl {
template <typename It, typename Sentinel> class Range {
public:
  template <typename R> Range(R &r) noexcept : m_it{std::begin(r)}, m_end{std::end(r)} {}

  Range(It it, Sentinel sentinel) noexcept
      : m_it{std::move(it)}, m_end{std::move(sentinel)} {}

  bool empty() const noexcept { return m_it == m_end; }

  std::size_t size() const noexcept { return m_end - m_it; }

  auto begin() const { return m_it; }
  auto end() const { return m_end; }

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
  Sentinel m_end;
};

template <typename R> Range(R &r)->Range<decltype(std::begin(r)), decltype(std::end(r))>;

LTL_MAKE_IS_KIND(Range, is_range, IsRange);

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

template <typename It, typename Sentinel, typename Predicate> struct FilterIterator {
  struct Attributes {
    Attributes(It &&it, It &&sentinelBegin, Sentinel &&sentinelEnd, Predicate &&predicate)
        : it{std::move(it)}, sentinelBegin{std::move(sentinelBegin)},
          sentinelEnd{std::move(sentinelEnd)}, predicate{std::move(predicate)} {}

    It it;
    const It sentinelBegin;
    const Sentinel sentinelEnd;
    Predicate predicate;
  };

public:
  using reference = typename std::iterator_traits<It>::reference;
  using difference_type = typename std::iterator_traits<It>::difference_type;
  using value_type = typename std::iterator_traits<It>::value_type;

  using iterator_category = std::conditional_t<
      std::is_base_of_v<std::bidirectional_iterator_tag,
                        typename std::iterator_traits<It>::iterator_category>,
      std::bidirectional_iterator_tag, std::forward_iterator_tag>;

  using pointer = AsPointer<reference>;

  FilterIterator() = default;

  FilterIterator(It &&it, It &&sentinelBegin, Sentinel &&sentinelEnd,
                 Predicate &&predicate) noexcept
      : m_attributes{std::in_place, std::move(it), std::move(sentinelBegin),
                     std::move(sentinelEnd), std::move(predicate)} {
    advanceUntilNext(IncrementTag{});
  }

  FilterIterator &operator++() noexcept {
    assert(m_attributes);
    ++m_attributes->it;
    advanceUntilNext(IncrementTag{});
    return *this;
  }

  FilterIterator &operator--() noexcept {
    assert(m_attributes);
    --m_attributes->it;
    advanceUntilNext(DecrementTag{});
    return *this;
  }

  reference operator*() noexcept {
    assert(m_attributes && m_attributes->it != m_attributes->sentinelEnd);
    return *m_attributes->it;
  }

  pointer operator->() noexcept {
    assert(m_attributes && m_attributes->it != m_attributes->sentinelEnd);
    return *m_attributes->it;
  }

  template <typename _Sentinel> bool operator!=(const _Sentinel &sentinel) {
    assert(m_attributes);
    return m_attributes->it != sentinel;
  }

  bool operator!=(const FilterIterator &it) {
    assert(it.m_attributes && m_attributes);
    return it.m_attributes->it == m_attributes->it;
  }

private:
  struct IncrementTag {};
  struct DecrementTag {};

  template <typename Tag> void advanceUntilNext(Tag) noexcept {
    assert(m_attributes);

    if_constexpr(type_v<Tag> == type_v<IncrementTag>) {
      while (m_attributes->it != m_attributes->sentinelEnd &&
             !m_attributes->predicate(*m_attributes->it)) {
        ++m_attributes->it;
      }
    }

    else {
      while (m_attributes->it != m_attributes->sentinelBegin &&
             !m_attributes->predicate(*m_attributes->it)) {
        --m_attributes->it;
      }
      assert(m_attributes->predicate(*m_attributes->it));
    }
  }

  std::optional<Attributes> m_attributes = std::nullopt;
};

template <typename It, typename Sentinel, typename Predicate>
FilterIterator(It &&, It &&, Sentinel &&, Predicate &&)
    ->FilterIterator<std::decay_t<It>, std::decay_t<Sentinel>, std::decay_t<Predicate>>;

template <typename F> struct FilterType { F f; };

template <typename F> auto filter(F &&f) { return FilterType<std::decay_t<F>>{FWD(f)}; }

template <typename R, typename F> auto operator|(R &&r, FilterType<F> f) {
  typed_static_assert(is_iterable(FWD(r)));

  if_constexpr(!is_range(FWD(r))) {
    typed_static_assert_msg(
        !is_rvalue_reference(FWD(r)),
        "An owning container cannot be passed through rvalue_reference");
  }

  return Range{FilterIterator{std::begin(FWD(r)), std::begin(FWD(r)), std::end(FWD(r)),
                              std::move(f.f)},
               std::end(FWD(r))};
}

} // namespace ltl
