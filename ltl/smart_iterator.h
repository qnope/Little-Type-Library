#pragma once
#include <iterator>

#include "strong_type.h"
#include "type_t.h"
#include "type_traits.h"

#include <cassert>
#include <optional>

namespace ltl {

namespace detail {
using Index = ltl::strong_type_t<std::size_t, struct IndexTag, Incrementable>;

#define DEFINE_ITERATOR(name)                                                  \
  using difference_type =                                                      \
      typename std::iterator_traits<name>::difference_type;                    \
  using pointer = typename std::iterator_traits<name>::pointer;                \
  using iterator_category = std::input_iterator_tag

template <typename It> class enumerate_range {
  struct enumerate_iterator {
    DEFINE_ITERATOR(It);
    using value_type = std::pair<const std::size_t,
                                 typename std::iterator_traits<It>::reference>;
    using reference = value_type;

    constexpr enumerate_iterator(It &&it)
        : index{std::size_t{0}}, it{std::move(it)} {}

    constexpr enumerate_iterator &operator++() {
      ++it;
      ++index;
      return *this;
    }

    constexpr auto operator*() { return value_type{index.get(), *it}; }

    constexpr bool operator!=(const enumerate_iterator &it2) const {
      return it != it2.it;
    }

    Index index;
    It it;
  };

public:
  constexpr enumerate_range(It &&begin, It &&end)
      : m_begin{std::move(begin)}, m_end{std::move(end)} {}

  [[nodiscard]] constexpr auto begin() { return m_begin; }
  [[nodiscard]] constexpr auto end() { return m_end; }

private:
  enumerate_iterator m_begin;
  enumerate_iterator m_end;
};

template <typename It, typename Filter> class filter_range {
  struct filter_iterator {
    DEFINE_ITERATOR(It);
    using value_type = typename std::iterator_traits<It>::value_type;
    using reference = typename std::iterator_traits<It>::reference;

    constexpr filter_iterator() = default;

    constexpr filter_iterator(It &&it, It &&end, Filter &&filter)
        : it{std::move(it)}, end{std::move(end)}, filter{std::move(filter)} {
      advance();
    }

    constexpr filter_iterator &operator++() {
      assert(it);
      ++(*it);
      advance();
      return *this;
    }

    constexpr reference operator*() { return **it; }

    constexpr void advance() {
      assert(it && end && filter);
      while (*it != *end && (*filter)(**it) == false)
        ++(*it);
    }

    constexpr bool operator!=(const filter_iterator &it2) const {
      if (it && it2.it)
        return *it != *it2.it;
      assert(it && end);
      return *it != *end;
    }

    std::optional<It> it;
    std::optional<It> end;
    std::optional<Filter> filter;
  };

public:
  constexpr filter_range(It &&begin, It &&end, Filter filter)
      : m_begin{std::move(begin), std::move(end), std::move(filter)} {}

  constexpr auto begin() { return m_begin; }
  constexpr auto end() { return m_end; }

private:
  filter_iterator m_begin;
  filter_iterator m_end;
};

template <typename It, typename MapFunction> class map_range {
  struct map_iterator {
    DEFINE_ITERATOR(It);
    using value_type = decltype(std::declval<MapFunction>()(
        std::declval<typename std::iterator_traits<It>::reference>()));
    using reference = value_type;

    constexpr map_iterator(It &&it) : it{std::move(it)} {}

    constexpr map_iterator(It &&it, MapFunction &&mapFunction)
        : it{std::move(it)}, mapFunction{std::move(mapFunction)} {}

    constexpr auto operator*() {
      assert(mapFunction);
      return (*mapFunction)(*it);
    }

    constexpr map_iterator &operator++() {
      ++it;
      return *this;
    }

    constexpr bool operator!=(const map_iterator &it2) const {
      return it != it2.it;
    }

    It it;
    std::optional<MapFunction> mapFunction;
  };

public:
  constexpr map_range(It &&begin, It &&end, MapFunction mapFunction)
      : m_begin{std::move(begin), std::move(mapFunction)}, m_end{std::move(
                                                               end)} {}

  constexpr auto begin() { return m_begin; }
  constexpr auto end() { return m_end; }

private:
  map_iterator m_begin;
  map_iterator m_end;
};

template <typename Container> class sorted_inserter_iterator {
public:
  using difference_type = void;
  using value_type = void;
  using pointer = void;
  using reference = void;
  using iterator_category = std::output_iterator_tag;

  sorted_inserter_iterator(Container &c) : m_container(&c) {}

  template <typename T> sorted_inserter_iterator &operator=(T &&object) {
    auto it = m_container->begin();
    auto end = m_container->end();

    while (it != end && *it < std::forward<T>(object)) {
      ++it;
    }

    m_container->insert(it, std::forward<T>(object));
    return *this;
  }

  sorted_inserter_iterator &operator++() { return *this; }
  sorted_inserter_iterator &operator*() { return *this; }

private:
  Container *m_container;
};

} // namespace detail

template <typename C, LTL_REQUIRE_T(is_iterable(type_v<C>))>
constexpr auto enumerate(C &&c) {
  return detail::enumerate_range<decltype(std::begin(c))>{std::begin(c),
                                                          std::end(c)};
}

template <typename Filter> constexpr auto filter(Filter filter) {
  return [f = std::move(filter)](auto &&c) {
    return detail::filter_range<decltype(std::begin(c)), Filter>{
        std::begin(c), std::end(c), f};
  };
}

template <typename MapFunction> constexpr auto map(MapFunction mapFunction) {
  return [f = std::move(mapFunction)](auto &&c) {
    return detail::map_range<decltype(std::begin(c)), MapFunction>{
        std::begin(c), std::end(c), f};
  };
}

template <typename Container> constexpr auto sorted_inserter(Container &c) {
  return ltl::detail::sorted_inserter_iterator<Container>{c};
}
} // namespace ltl
