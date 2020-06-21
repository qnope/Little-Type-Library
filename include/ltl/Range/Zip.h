#pragma once

#include "../tuple_algos.h"
#include "BaseIterator.h"
#include "Range.h"

namespace ltl {
template <typename... Iterators>
struct ZipIterator : BaseIterator<ZipIterator<Iterators...>, tuple_t<Iterators...>, Nothing> {
    using reference = tuple_t<typename std::iterator_traits<Iterators>::reference...>;

    using BaseIterator<ZipIterator<Iterators...>, tuple_t<Iterators...>, Nothing>::BaseIterator;

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

    reference operator*() const { return TO_VARIADIC(this->m_it, xs, return reference{*xs...}); }
};

namespace details {
using std::begin;
using std::end;

template <typename... Containers>
auto build_begin_zip_iterator(Containers &&... containers) {
    return ZipIterator<decltype(begin(FWD(containers)))...>{tuple_t{begin(FWD(containers))...},
                                                            tuple_t{begin(FWD(containers))...},
                                                            tuple_t{end(FWD(containers))...}, Nothing{}};
}

template <typename... Containers>
auto build_end_zip_iterator(Containers &&... containers) {
    return ZipIterator<decltype(end(FWD(containers)))...>{tuple_t{end(FWD(containers))...}};
}
} // namespace details

template <typename... Containers>
class ZipRange : public AbstractRange<ZipRange<Containers...>> {
  public:
    ZipRange(Containers... containers) noexcept : m_containers{FWD(containers)...} {}

    auto begin() const noexcept { return std::move(m_containers)(lift(details::build_begin_zip_iterator)); }

    auto end() const noexcept { return std::move(m_containers)(lift(details::build_end_zip_iterator)); }

  private:
    ltl::tuple_t<Containers...> m_containers;
};

template <typename... Containers>
auto zip(Containers &&... containers) {
    constexpr auto types = type_list_v<Containers...>;
    using std::size;
    typed_static_assert(!types.isEmpty);
    typed_static_assert_msg(all_of_type(types, is_iterable), "Zip operations must be used with containers");

    assert(FROM_VARIADIC(FWD(containers))([](auto &&c1, auto &&... cs) {
        return (true && ... && (std::size_t(size(FWD(c1))) == std::size_t(size(FWD(cs)))));
    }));

    return ZipRange<Containers...>{FWD(containers)...};
}

} // namespace ltl
