#pragma once

#include "BaseIterator.h"
#include "Range.h"

namespace ltl {
template <typename It, typename Getter>
class GroupByIterator : public BaseIterator<GroupByIterator<It, Getter>, It, Getter, false> {
    friend BaseIterator<GroupByIterator, It, Getter, false>;

  public:
    using key = decltype(ltl::invoke(std::declval<Getter &>(), *std::declval<It>()));
    using range_type = Range<It>;

    using reference = tuple_t<key, range_type>;
    DECLARE_EVERYTHING_BUT_REFERENCE(typename std::iterator_traits<It>::iterator_category)

    using BaseIterator<GroupByIterator<It, Getter>, It, Getter, false>::BaseIterator;

    GroupByIterator(It it, It sentinelBegin, It sentinelEnd, Getter function) :
        BaseIterator<GroupByIterator<It, Getter>, It, Getter, false>{std::move(it), std::move(sentinelBegin),
                                                                     std::move(sentinelEnd), std::move(function)} {
        if (this->m_it != this->m_sentinelEnd) {
            m_nextIterator = computeNextIterator();
        }
        m_previousIterator = this->m_it;
    }

    GroupByIterator &operator++() noexcept {
        assert(this->m_it != this->m_sentinelEnd);
        m_previousIterator = this->m_it;
        this->m_it = this->m_nextIterator;
        if (this->m_it != this->m_sentinelEnd) {
            this->m_nextIterator = computeNextIterator();
        }
        return *this;
    }

    GroupByIterator &operator--() noexcept {
        assert(this->m_it != this->m_sentinelBegin);
        m_nextIterator = this->m_it;
        this->m_it = this->m_previousIterator;
        if (this->m_it != this->m_sentinelBegin) {
            this->m_previousIterator = computePreviousIterator();
        }
        return *this;
    }

    friend std::size_t operator-(const GroupByIterator &b, GroupByIterator a) {
        std::size_t res = 0;
        while (a != b) {
            ++res;
            ++a;
        }
        return res;
    }

    reference operator*() const noexcept {
        return {this->m_function(*this->m_it), range_type{this->m_it, m_nextIterator}};
    }

  private:
    auto computeNextIterator() {
        decltype(auto) value = this->m_function(*this->m_it);
        return std::find_if_not(std::next(this->m_it), this->m_sentinelEnd,
                                [&value, this](const auto &x) { return this->m_function(x) == value; });
    }

    template <typename T>
    auto computePreviousIterator(It it, const T &value) {
        if (it == this->m_sentinelBegin)
            return it;

        auto prev = std::prev(it);

        if (this->m_function(*prev) != value)
            return it;

        return computePreviousIterator(prev, value);
    }

    auto computePreviousIterator() {
        auto it = std::prev(this->m_it);
        decltype(auto) value = this->m_function(*it);
        return computePreviousIterator(std::move(it), value);
    }

  private:
    It m_previousIterator;
    It m_nextIterator;
};

template <typename F>
struct GroupByType {
    F f;
};
template <typename F>
auto group_by(F &&f) {
    return GroupByType<std::decay_t<F>>{FWD(f)};
}

template <typename F>
struct is_chainable_operation<GroupByType<F>> : true_t {};

template <typename T1, typename F, requires_f(IsIterableRef<T1>)>
constexpr decltype(auto) operator|(T1 &&a, GroupByType<F> b) {
    using std::begin;
    using std::end;
    using it = decltype(begin(FWD(a)));
    return Range{GroupByIterator<it, std::decay_t<decltype(std::move(b.f))>>{begin(FWD(a)), begin(FWD(a)), end(FWD(a)),
                                                                             std::move(b.f)},
                 GroupByIterator<it, std::decay_t<decltype(std::move(b.f))>>{end(FWD(a))}};
}
} // namespace ltl
