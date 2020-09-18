#pragma once

#include "../algos.h"
#include "../concept.h"
#include "../functional.h"

namespace ltl {
namespace actions {

constexpr struct Sort : AbstractAction {
} sort;

template <typename F>
struct SortBy : AbstractAction {
    SortBy(F &&f) : f{std::move(f)} {}
    F f;
};

template <typename F>
constexpr auto sort_by(F f) {
    return SortBy<F>{std::move(f)};
}

constexpr struct Unique : AbstractAction {
} unique;

constexpr struct Reverse : AbstractAction {
} reverse;

template <typename T>
struct Find {
    T elem;
};

template <typename T>
struct FindValue {
    T elem;
};

template <typename T>
struct FindPtr {
    T elem;
};

template <typename F>
struct FindIf {
    F f;
};

template <typename F>
struct FindIfValue {
    F f;
};

template <typename F>
struct FindIfPtr {
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
    auto foo = compose(std::move(fs)...);
    return FindIf<decltype(foo)>{compose(std::move(fs)...)};
}
template <typename... Fs>
constexpr auto find_if_value(Fs... fs) {
    auto foo = compose(std::move(fs)...);
    return FindIfValue<decltype(foo)>{compose(std::move(fs)...)};
}
template <typename... Fs>
constexpr auto find_if_ptr(Fs... fs) {
    auto foo = compose(std::move(fs)...);
    return FindIfPtr<decltype(foo)>{compose(std::move(fs)...)};
}

template <typename Action1, typename Action2, requires_f(IsAction<Action1>), requires_f(IsAction<Action2>)>
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
auto operator|(C &c, FindValue<T> e) {
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
auto operator|(C &c, FindIfValue<F> e) {
    return ::ltl::find_if_value(c, e.f);
}

template <typename C, typename F, requires_f(ltl::IsIterable<C>)>
auto operator|(C &c, FindIfPtr<F> e) {
    return ::ltl::find_if_ptr(c, e.f);
}

template <typename C, typename Action, requires_f(ltl::IsIterable<C>), requires_f(IsAction<Action>)>
auto operator|(C c, Action a) {
    c |= a;
    return c;
}

template <typename C, typename... Actions, requires_f(ltl::IsIterable<C>)>
auto &operator|=(C &c, ltl::tuple_t<Actions...> actions) {
    return actions([&c](const auto &... xs) -> C & {
        ((c |= xs), ...);
        return c;
    });
}

template <typename C, typename... Actions, requires_f(ltl::IsIterable<C>),
          requires_f((true && ... && IsAction<Actions>))>
auto operator|(C c, ltl::tuple_t<Actions...> actions) {
    return actions([c = std::move(c)](const auto &... xs) mutable {
        ((c |= xs), ...);
        return c;
    });
}

} // namespace actions
} // namespace ltl
