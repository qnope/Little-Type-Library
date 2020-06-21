#pragma once

#include <iterator>

#include "../Tuple.h"
#include "../crtp.h"
#include "../traits.h"

namespace ltl {

using std::begin;
using std::end;

namespace details {
template <typename R>
auto callBegin(R &&r) {
    return begin(FWD(r));
}
template <typename R>
auto callEnd(R &&r) {
    return end(FWD(r));
}
} // namespace details

template <typename Derived>
class AbstractRange {
    ENABLE_CRTP(Derived)
  public:
    bool empty() const noexcept { return underlying().begin() == underlying().end(); }

    std::size_t size() const noexcept { return std::distance(underlying().begin(), underlying().end()); }

    decltype(auto) operator[](std::size_t idx) const noexcept {
        assert(idx < size());
        return *(underlying().begin() + idx);
    }

    decltype(auto) front() const noexcept {
        assert(!empty());
        return *underlying().begin();
    }

    decltype(auto) back() const noexcept {
        assert(!empty());
        return *(underlying().begin() + std::size_t{size() - 1});
    }

    template <typename T>
    operator T() const noexcept {
        return T(underlying().begin(), underlying().end());
    }
};

template <typename It>
class Range : public AbstractRange<Range<It>> {
  public:
    template <typename R>
    Range(R &r) noexcept : m_it{details::callBegin(r)}, m_end{details::callEnd(r)} {}

    Range(It it, It end) noexcept : m_it{std::move(it)}, m_end{std::move(end)} {}

    auto begin() const noexcept { return m_it; }
    auto end() const noexcept { return m_end; }

  private:
    It m_it;
    It m_end;
};

template <typename Container, typename... Operations>
class OwningRange : public AbstractRange<OwningRange<Container, Operations...>> {
    using range_type = decltype(std::declval<Container &>() | std::declval<ltl::tuple_t<Operations...>>());

  public:
    OwningRange(Container container, Operations... operations) noexcept :
        m_container{std::move(container)}, //
        m_operations{FWD(operations)...},  //
        m_range{m_container | m_operations} {}

    auto begin() const noexcept { return m_range.begin(); }

    auto end() const noexcept { return m_range.end(); }

    template <typename NewOperation>
    auto add_operation(NewOperation newOperation) && {
        return std::move(m_operations)([this, &newOperation](auto &&... ops) mutable {
            return OwningRange<Container, Operations..., NewOperation>{std::move(m_container), FWD(ops)...,
                                                                       std::forward<NewOperation>(newOperation)};
        });
    }

  private:
    Container m_container;
    tuple_t<Operations...> m_operations;
    range_type m_range;
};

template <typename R>
Range(R &r) -> Range<decltype(std::begin(r))>;

template <typename R>
auto begin(const AbstractRange<R> &r) noexcept {
    return static_cast<const R &>(r).begin();
}

template <typename R>
auto end(const AbstractRange<R> &r) noexcept {
    return static_cast<const R &>(r).end();
}

template <typename R>
auto size(const AbstractRange<R> &r) noexcept {
    return static_cast<const R &>(r).size();
}

LTL_MAKE_IS_KIND(Range, is_range, IsRange, typename);
LTL_MAKE_IS_KIND(OwningRange, is_owning_range, IsOwningRange, typename);
} // namespace ltl
