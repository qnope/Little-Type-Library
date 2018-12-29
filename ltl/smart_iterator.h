#pragma once
#include <iterator>

#include "strong_type.h"
#include "type_t.h"
#include "type_traits.h"

namespace ltl {
using Index = strong_type_t<std::size_t, struct IndexTag, Incrementable>;

template <typename It> class enumerate_range {
  struct enumerate_iterator {
    constexpr enumerate_iterator(It it) : index{std::size_t{0}}, it{it} {}

    constexpr enumerate_iterator &operator++() {
      ++it;
      ++index;
      return *this;
    }

    constexpr auto operator*() {
      return std::pair<std::size_t, decltype(*it)>{index.get(), *it};
    }

    constexpr bool operator!=(const enumerate_iterator &it2) {
      return it != it2.it;
    }

    Index index;
    It it;
  };

public:
  constexpr enumerate_range(It begin, It end) : m_begin{begin}, m_end{end} {}

  [[nodiscard]] constexpr auto begin() { return m_begin; }
  [[nodiscard]] constexpr auto end() { return m_end; }

private:
  enumerate_iterator m_begin;
  enumerate_iterator m_end;
};

template <typename C, LTL_REQUIRE_T(is_iterable(type_v<C>))>
constexpr auto enumerate(C &&c) {
  return enumerate_range<decltype(std::begin(c))>{std::begin(c), std::end(c)};
}

} // namespace ltl
