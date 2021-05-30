/**
 * @file Zip.h
 */
#pragma once

#include "ltl/tuple_algos.h"
#include "BaseIterator.h"
#include "Range.h"

namespace ltl {
template <typename... Iterators>
struct ZipIterator :
    BaseIterator<ZipIterator<Iterators...>, tuple_t<Iterators...>>,
    IteratorOperationByIterating<ZipIterator<Iterators...>>,
    IteratorSimpleComparator<ZipIterator<Iterators...>> {
    using reference = tuple_t<typename std::iterator_traits<Iterators>::reference...>;

    using BaseIterator<ZipIterator, tuple_t<Iterators...>>::BaseIterator;

    DECLARE_EVERYTHING_BUT_REFERENCE(std::common_type_t<get_iterator_category<Iterators>...>);

    ZipIterator &operator++() {
        this->m_it([](auto &...xs) { (++xs, ...); });
        return *this;
    }

    ZipIterator &operator--() {
        this->m_it([](auto &...xs) { (--xs, ...); });
        return *this;
    }

    reference operator*() const {
        return this->m_it([](auto &&...xs) { return reference{*FWD(xs)...}; });
    }
};

namespace details {
using std::begin;
using std::end;

template <typename... Containers>
auto build_begin_zip_iterator(Containers &...containers) {
    return ZipIterator<decltype(begin(containers))...>{tuple_t{begin(containers)...}};
}

template <typename... Containers>
auto build_end_zip_iterator(Containers &...containers) {
    auto it = build_begin_zip_iterator(containers...);
    while (it.m_it([&](const auto &...xs) { return (... && (xs != end(containers))); }))
        ++it;
    return it;
}
} // namespace details

template <typename... Containers>
class ZipRange : public AbstractRange<ZipRange<Containers...>> {
  public:
    ZipRange(Containers... containers) noexcept : m_containers{FWD(containers)...} {}

    auto begin() const noexcept { return m_containers(lift(details::build_begin_zip_iterator)); }

    auto end() const noexcept { return m_containers(lift(details::build_end_zip_iterator)); }

  private:
    ltl::tuple_t<Containers...> m_containers;
};

template <typename... Containers>
auto zip(Containers &&...containers) {
    using std::size;
    static_assert(sizeof...(Containers) > 0);
    static_assert((IsIterable<Containers> && ...), "Zip operations must be used with containers");

    return ZipRange<Containers...>{FWD(containers)...};
}

} // namespace ltl
