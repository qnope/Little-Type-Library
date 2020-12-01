#pragma once

#include "Tuple.h"

namespace ltl {
//////////////////////////////// Y_Combinator /////////////////////////////////
template <typename F>
struct fix : F {
    template <typename... Args>
    constexpr auto operator()(Args &&... args) const
        -> decltype(std::declval<const F &>()(std::declval<const fix &>(), std::declval<Args>()...)) {
        return static_cast<const F &>(*this)(*this, FWD(args)...);
    }

    template <typename... Args>
    constexpr auto operator()(Args &&... args)
        -> decltype(std::declval<F &>()(std::declval<fix &>(), std::declval<Args>()...)) {
        return static_cast<F &>(*this)(*this, FWD(args)...);
    }
};
template <typename F>
fix(F)->fix<F>;

template <typename F, typename... Args>
constexpr auto report_call(F f, Args... xs) {
    return [f = std::move(f), xs...](auto &&... _ys) -> decltype(auto) { return ltl::invoke(f, xs..., FWD(_ys)...); };
}

template <typename F, typename... Args>
constexpr decltype(auto) curry(F f, Args &&... args) {
    if constexpr (std::is_invocable_v<F, Args...>) {
        return ltl::invoke(f, FWD(args)...);
    } else {
        return report_call(lift(curry), std::move(f), FWD(args)...);
    }
}

template <typename F>
constexpr auto compose(F f) {
    return f;
}

template <typename F, typename... Fs>
constexpr auto compose(F f, Fs... fs) {
    return [f, fs...](auto &&... xs) -> decltype(auto) { //
        return ltl::fast_invoke(compose(fs...), ltl::fast_invoke(f, FWD(xs)...));
    };
}

template <typename F>
constexpr auto not_(F f) {
    return [f = std::move(f)](auto &&... xs) { return !ltl::fast_invoke(f, FWD(xs)...); };
}

template <typename... Fs>
constexpr auto or_(Fs... fs) {
    return [fs...](auto &&... xs) { return (false_v || ... || (fs(FWD(xs)...))); };
}

template <typename... Fs>
constexpr auto and_(Fs... fs) {
    return [fs...](auto &&... xs) { return (true_v && ... && (fs(FWD(xs)...))); };
}

template <typename T>
constexpr auto less_than(T t) {
    return [t = std::move(t)](auto x) { return x < t; };
}

template <typename T>
constexpr auto less_than_equal(T t) {
    return [t = std::move(t)](const auto &x) { return x <= t; };
}

template <typename T>
constexpr auto greater_than(T t) {
    return [t = std::move(t)](const auto &x) { return x > t; };
}

template <typename T>
constexpr auto greater_than_equal(T t) {
    return [t = std::move(t)](const auto &x) { return x >= t; };
}

template <typename T>
constexpr auto equal_to(T t) {
    return [t = std::move(t)](const auto &x) { return x == t; };
}

template <typename T>
constexpr auto not_equal_to(T t) {
    return [t = std::move(t)](const auto &x) { return x != t; };
}

template <typename F>
constexpr auto unzip(F f) {
    return [f = std::move(f)](auto &&tuple) { return apply(f, FWD(tuple)); };
}

namespace detail {
template <typename T>
struct construct_impl {
    template <typename... Ts>
    constexpr auto operator()(Ts &&... ts) -> decltype(T{std::declval<Ts>()...}) {
        return T{FWD(ts)...};
    }
};
} // namespace detail

template <typename T, typename... Args>
constexpr auto construct(Args &&... args) noexcept {
    using namespace ltl;

    return [args...](auto &&... xs) { return curry(detail::construct_impl<T>{}, args..., FWD(xs)...); };
}

template <typename T, typename... Tuple>
constexpr auto construct_with_tuple(Tuple &&... tuple) noexcept {
    static_assert(sizeof...(Tuple) <= 1, "Must have 0 or one tuple");
    return curry(unzip(construct<T>()), FWD(tuple)...);
}

constexpr auto identity = [](auto &&t) -> remove_rvalue_reference_t<decltype(FWD(t))> { return FWD(t); };
constexpr auto id_copy = [](auto x) { return x; };

constexpr auto byAscending = [](auto f) {
    return [f = std::move(f)](const auto &x, const auto &y) noexcept { //
        return ltl::fast_invoke(f, x) < ltl::fast_invoke(f, y);
    };
};

constexpr auto byDescending = [](auto f) {
    return [f = std::move(f)](const auto &x, const auto &y) noexcept { //
        return ltl::fast_invoke(f, x) > ltl::fast_invoke(f, y);
    };
};

} // namespace ltl
