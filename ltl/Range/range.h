#pragma once

#include <iterator>

#include "../ltl.h"

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

template <typename It> auto begin(const Range<It> &r) noexcept {
  return r.begin();
}
template <typename It> auto end(const Range<It> &r) noexcept { return r.end(); }
template <typename It> std::size_t size(const Range<It> &r) noexcept {
  return r.size();
}

LTL_MAKE_IS_KIND(Range, is_range, IsRange, typename);
} // namespace ltl
