#pragma once

#include "../algos.h"
#include "../concept.h"

namespace ltl {
namespace actions {

struct AbstractAction {};

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

constexpr auto is_action = ltl::is_derived_from(ltl::type_v<AbstractAction>);

template <typename T>
constexpr bool IsAction = decltype(is_action(std::declval<T>()))::value;

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
