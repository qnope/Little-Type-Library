#pragma once

#include "../algos.h"
#include "../concept.h"
#include "../functional.h"

#include "Taker.h"

namespace ltl {
namespace actions {

using std::begin;
using std::end;

struct AbstractAction {};
struct AbstractModifyingAction : AbstractAction {};

template <typename T>
constexpr bool IsAction = std::is_base_of_v<AbstractAction, std::decay_t<T>>;

template <typename T>
constexpr bool IsModifyingAction = std::is_base_of_v<AbstractModifyingAction, std::decay_t<T>>;

constexpr struct Sort : AbstractModifyingAction {
} sort;

template <typename F>
struct SortBy : AbstractModifyingAction {
    SortBy(F &&f) : f{std::move(f)} {}
    F f;
};

template <typename F>
constexpr auto sort_by(F f) {
    return SortBy<F>{std::move(f)};
}

constexpr struct Unique : AbstractModifyingAction {
} unique;

constexpr struct Reverse : AbstractModifyingAction {
} reverse;

template <typename T>
struct Find : AbstractAction {
    Find(T &&t) : elem{static_cast<T &&>(t)} {}
    T elem;
};

template <typename T>
struct FindValue : AbstractAction {
    FindValue(T &&t) : elem{static_cast<T &&>(t)} {}
    T elem;
};

template <typename T>
struct FindPtr : AbstractAction {
    FindPtr(T &&t) : elem{static_cast<T &&>(t)} {}
    T elem;
};

template <typename F>
struct FindIf : AbstractAction {
    FindIf(F &&f) : f{static_cast<F &&>(f)} {}

    F f;
};

template <typename F>
struct FindIfValue : AbstractAction {
    FindIfValue(F &&f) : f{static_cast<F &&>(f)} {}
    F f;
};

template <typename F>
struct FindIfPtr : AbstractAction {
    FindIfPtr(F &&f) : f{static_cast<F &&>(f)} {}
    F f;
};

template <typename T>
constexpr auto find(T &&e) {
    return Find<T>{FWD(e)};
}
template <typename T>
constexpr auto find_value(T &&e) {
    return FindValue<T>{FWD(e)};
}
template <typename T>
constexpr auto find_ptr(T &&e) {
    return FindPtr<T>{FWD(e)};
}

template <typename... Fs>
constexpr auto find_if(Fs... fs) {
    return FindIf{compose(std::move(fs)...)};
}
template <typename... Fs>
constexpr auto find_if_value(Fs... fs) {
    return FindIfValue{compose(std::move(fs)...)};
}
template <typename... Fs>
constexpr auto find_if_ptr(Fs... fs) {
    return FindIfPtr{compose(std::move(fs)...)};
}

template <typename D>
struct JoinWith : AbstractAction {
    JoinWith(D &&d) : d{static_cast<D &&>(d)} {}
    D d;
};

template <typename D>
constexpr auto join_with(D &&d) {
    return JoinWith<D>{FWD(d)};
}

template <typename T, typename F>
struct Accumulate : AbstractAction {
    Accumulate(T &&init, F &&f) : init{FWD(init)}, f{static_cast<F &&>(f)} {}
    T init;
    F f;
};

template <typename T, typename F = std::plus<>>
constexpr auto accumulate(T &&init, F f = F{}) {
    return Accumulate<T, F>{FWD(init), std::move(f)};
}

constexpr struct Sum : AbstractAction {
} sum;

template <typename Action1, typename Action2, requires_f(IsAction<Action1> &&IsAction<Action2>)>
constexpr auto operator|(Action1 a, Action2 b) {
    return ltl::tuple_t{std::move(a), std::move(b)};
}

template <typename... Actions, typename Action, requires_f(IsAction<Action>)>
constexpr auto operator|(ltl::tuple_t<Actions...> a, Action b) {
    return a.push_back(std::move(b));
}

template <typename... Actions, typename Action, requires_f(IsAction<Action>)>
constexpr auto operator|(Action a, ltl::tuple_t<Actions...> b) {
    return b.push_front(std::move(a));
}

template <typename C, requires_f(ltl::IsIterable<C>)>
auto &operator|=(C &c, Sort) {
    return ltl::sort(c);
}

template <typename C, typename F, requires_f(ltl::IsIterable<C>)>
auto &operator|=(C &c, SortBy<F> sortBy) {
    return ltl::sort(c, sortBy.f);
}

template <typename C, requires_f(ltl::IsIterable<C>)>
auto &operator|=(C &c, Unique) {
    c.erase(ltl::unique(c), end(c));
    return c;
}

template <typename C, requires_f(ltl::IsIterable<C>)>
auto &operator|=(C &c, Reverse) {
    ::ltl::reverse(c);
    return c;
}

template <typename C, typename T, requires_f(ltl::IsIterable<C>)>
auto operator|(C &c, Find<T> e) {
    return ::ltl::find(c, e.elem);
}

template <typename C, typename T, requires_f(ltl::IsIterable<C>)>
auto operator|(const C &c, FindValue<T> e) {
    return ::ltl::find_value(c, e.elem);
}

template <typename C, typename T, requires_f(ltl::IsIterable<C>)>
auto operator|(C &c, FindPtr<T> e) {
    return ::ltl::find_ptr(c, e.elem);
}

template <typename C, typename F, requires_f(ltl::IsIterable<C>)>
auto operator|(C &c, FindIf<F> e) {
    return ::ltl::find_if(c, e.f);
}

template <typename C, typename F, requires_f(ltl::IsIterable<C>)>
auto operator|(const C &c, FindIfValue<F> e) {
    return ::ltl::find_if_value(c, e.f);
}

template <typename C, typename F, requires_f(ltl::IsIterable<C>)>
auto operator|(C &c, FindIfPtr<F> e) {
    return ::ltl::find_if_ptr(c, e.f);
}

template <typename C, typename D, requires_f(ltl::IsIterable<C>)>
auto operator|(const C &c, JoinWith<D> d) -> std::decay_t<decltype(*c.begin())> {
    std::decay_t<decltype(*begin(c))> result{};
    if (c.empty()) {
        return result;
    } else {
        return std::accumulate(std::next(begin(c)), end(c), *c.begin(), [&d](auto init, auto other) { //
            return std::move(init) + d.d + std::move(other);
        });
    }
}

template <typename C, typename T, typename F, requires_f(ltl::IsIterable<C>)>
auto operator|(const C &c, Accumulate<T, F> a) {
    return ltl::accumulate(c, FWD(a.init), a.f);
}

template <typename C, requires_f(ltl::IsIterable<C>)>
auto operator|(const C &c, Sum) {
    return ltl::accumulate(c, std::decay_t<decltype(*begin(c))>{});
}

template <typename C, typename Action, requires_f(ltl::IsIterable<C> &&IsModifyingAction<Action>)>
auto operator|(C c, Action a) {
    c |= a;
    return c;
}

template <typename C, typename... Actions, requires_f(ltl::IsIterable<C>)>
auto &operator|=(C &c, ltl::tuple_t<Actions...> actions) {
    return actions([&c](const auto &...xs) -> C & {
        ((c |= xs), ...);
        return c;
    });
}

template <typename C, typename... Actions, requires_f(ltl::IsIterable<C>),
          requires_f((true && ... && IsAction<Actions>))>
auto operator|(C c, ltl::tuple_t<Actions...> actions) {
    return actions([c = std::move(c)](const auto &...xs) mutable { return (c | ... | xs); });
}

} // namespace actions
} // namespace ltl
