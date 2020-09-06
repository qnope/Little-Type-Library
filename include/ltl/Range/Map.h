#pragma once

#include "../concept.h"
#include "../functional.h"
#include "../optional_type.h"

#include "Join.h"
#include "Range.h"

namespace ltl {
template <typename It, typename Function>
struct MapIterator : BaseIterator<MapIterator<It, Function>, It, Function> {
    using reference = std::invoke_result_t<Function, typename std::iterator_traits<It>::reference>;
    DECLARE_EVERYTHING_BUT_REFERENCE(typename std::iterator_traits<It>::iterator_category)

    using BaseIterator<MapIterator<It, Function>, It, Function>::BaseIterator;

    reference operator*() const { return this->m_function(*this->m_it); }
};

template <typename F>
struct MapType {
    F f;
};
template <typename F>
constexpr auto map(F &&f) {
    return MapType<std::decay_t<F>>{FWD(f)};
}
template <typename... Fs>
constexpr auto map(Fs... fs) {
    return map(compose(std::move(fs)...));
}

template <typename F>
struct is_chainable_operation<MapType<F>> : true_t {};

template <typename T1, typename F, requires_f(IsIterableRef<T1>)>
constexpr decltype(auto) operator|(T1 &&a, MapType<F> b) {
    using std::begin;
    using std::end;
    using it = decltype(begin(FWD(a)));
    return Range{MapIterator<it, std::decay_t<decltype(std::move(b.f))>>{begin(FWD(a)), begin(FWD(a)), end(FWD(a)),
                                                                         std::move(b.f)},
                 MapIterator<it, std::decay_t<decltype(std::move(b.f))>>{end(FWD(a))}};
}

template <typename T1, typename F, requires_f(IsOptional<T1>)>
constexpr decltype(auto) operator|(T1 &&a, MapType<F> b) {
    if (a)
        return std::make_optional(ltl::invoke(std::move(b.f), *FWD(a)));
    return decltype(std::make_optional(ltl::invoke(std::move(b.f), *FWD(a)))){};
}

template <typename T1, typename F, requires_f(IsOptionalType<T1>)>
constexpr decltype(auto) operator|(T1 &&a, MapType<F> b) {
    if_constexpr(a.has_value) { //
        return ltl::optional_type{ltl::invoke(std::move(b.f), *a)};
    }
    else return ltl::nullopt_type;
}

template <typename T1, typename F, requires_f(IsIterable<T1>)>
constexpr decltype(auto) operator>>(T1 &&a, MapType<F> b) {
    return FWD(a) | std::move(b) | join;
}

template <typename T1, typename F, requires_f(IsOptional<T1>)>
constexpr decltype(auto) operator>>(T1 &&a, MapType<F> b) {
    typed_static_assert_msg(is_optional(ltl::invoke(std::move(b.f), *a)),
                            "With >> notation, function must return optional");
    if (a)
        return ltl::invoke(std::move(b.f), *FWD(a));
    return decltype(ltl::invoke(std::move(b.f), *FWD(a))){};
}

template <typename T1, typename F, requires_f(IsOptionalType<T1>)>
constexpr decltype(auto) operator>>(T1 &&a, MapType<F> b) {
    if_constexpr(a.has_value) { //
        typed_static_assert_msg(is_optional_type(ltl::invoke(std::move(b.f), *a)),
                                "With >> notation, function must return optional_type");
        return ltl::invoke(std::move(b.f), *a);
    }
    else {
        return ltl::nullopt_type;
    }
}

template <typename T>
auto make_move_range(T &&t) {
    auto move = [](auto &x) -> std::decay_t<decltype(x)> { //
        return std::move(x);
    };
    return FWD(t) | ltl::map(move);
}

} // namespace ltl
