#pragma once
#include <iterator>

#include "strong_type.h"
#include "type_t.h"
#include "type_traits.h"

namespace ltl {

namespace detail {
using Index = ltl::strong_type_t<std::size_t, struct IndexTag, Incrementable>;

template <typename It> class enumerate_range {
  struct enumerate_iterator {
    constexpr enumerate_iterator(It it)
        : index{std::size_t{0}}, it{std::move(it)} {}

    constexpr enumerate_iterator &operator++() {
      ++it;
      ++index;
      return *this;
    }

    constexpr auto operator*() {
      return std::pair<const std::size_t, decltype(*it)>{index.get(), *it};
    }

    constexpr bool operator!=(const enumerate_iterator &it2) const {
      return it != it2.it;
    }

    Index index;
    It it;
  };

public:
  template <typename _It>
  constexpr enumerate_range(_It &&begin, _It &&end)
      : m_begin{std::forward<_It>(begin)}, m_end{std::forward<_It>(end)} {}

  [[nodiscard]] constexpr auto begin() { return m_begin; }
  [[nodiscard]] constexpr auto end() { return m_end; }

private:
  enumerate_iterator m_begin;
  enumerate_iterator m_end;
};

template <typename It, typename Filter> class filter_range {
  struct filter_iterator {
    constexpr filter_iterator(It it, It end, Filter filter)
        : it{std::move(it)}, end{std::move(end)}, filter{std::move(filter)} {
      advance();
    }

    constexpr filter_iterator &operator++() {
      it++;
      advance();
      return *this;
    }

    constexpr decltype(auto) operator*() { return *it; }

    constexpr void advance() {
      while (it != end && filter(*it) == false)
        ++it;
    }

    constexpr bool operator!=(const filter_iterator &it2) const {
      return it != it2.it;
    }

    Filter filter;
    It it;
    It end;
  };

public:
  constexpr filter_range(It begin, It end, Filter filter)
      : m_begin{std::move(begin), end, filter}, m_end{end, end, filter} {}

  constexpr auto begin() { return m_begin; }
  constexpr auto end() { return m_end; }

private:
  filter_iterator m_begin;
  filter_iterator m_end;
};

template <typename It, typename MapFunction> class map_range {
  struct map_iterator {
    constexpr map_iterator(It it, MapFunction mapFunction)
        : it{std::move(it)}, mapFunction{std::move(mapFunction)} {}

    constexpr auto operator*() { return mapFunction(*it); }

    constexpr map_iterator &operator++() {
      ++it;
      return *this;
    }

    constexpr bool operator!=(const map_iterator &it2) const {
      return it != it2.it;
    }

    It it;
    MapFunction mapFunction;
  };

public:
  template <typename _It>
  constexpr map_range(_It begin, _It end, MapFunction mapFunction)
      : m_begin{std::forward<_It>(begin), mapFunction}, m_end{std::forward<_It>(
                                                                  end),
                                                              mapFunction} {}
  constexpr auto begin() { return m_begin; }
  constexpr auto end() { return m_end; }

private:
  map_iterator m_begin;
  map_iterator m_end;
};

} // namespace detail

template <typename C, LTL_REQUIRE_T(is_iterable(type_v<C>))>
constexpr auto enumerate(C &&c) {
  return detail::enumerate_range<decltype(std::begin(c))>{std::begin(c),
                                                          std::end(c)};
}

template <typename Filter> constexpr auto filter(Filter filter) {
  return [filter](auto &c) {
    return detail::filter_range<decltype(std::begin(c)), Filter>{
        std::begin(c), std::end(c), std::move(filter)};
  };
}

template <typename MapFunction> constexpr auto map(MapFunction mapFunction) {
  return [mapFunction](auto &c) {
    return detail::map_range<decltype(std::begin(c)), MapFunction>{
        std::begin(c), std::end(c), std::move(mapFunction)};
  };
}
} // namespace ltl
