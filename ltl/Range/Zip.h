#pragma once

#include "../Tuple.h"
#include "BaseIterator.h"
#include "Range.h"

namespace ltl {
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

  reference operator*() const {
    return TO_VARIADIC(this->m_it, xs, return reference{*xs...});
  }

  constexpr void advanceUntilNext(...) {}
};

template <typename... Containers> auto zip(Containers &&... containers) {
  constexpr auto types = type_list_v<Containers...>;
  using std::size;
  typed_static_assert(!types.isEmpty);
  typed_static_assert_msg(all_of_type(types, is_iterable),
                          "Zip operations must be used with containers");

  assert(FROM_VARIADIC(FWD(containers))([](auto &&c1, auto &&... cs) {
    return (true && ... && (size(FWD(c1)) == size(FWD(cs))));
  }));

  return Range{ZipIterator<decltype(std::begin(FWD(containers)))...>{
                   tuple_t{std::begin(FWD(containers))...},
                   tuple_t{std::begin(FWD(containers))...},
                   tuple_t{std::end(FWD(containers))...}, Nothing{}},
               ZipIterator<decltype(std::begin(FWD(containers)))...>{
                   tuple_t{std::end(FWD(containers))...}}};
}

} // namespace ltl
