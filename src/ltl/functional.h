/**
 * @file functional.h
 */
#pragma once

#include "Tuple.h"

namespace ltl {

/**
 *\defgroup Function The functional group
 *@{
 */

template <typename F>
/**
 * @brief Fix represents Y_Combinator, it is useful for recursive lambdas
 *
 * In order to use fix, the lambda must receive as first argument itself.
 * Let's say one wants to implement a factorial in a recursive way using lambda, it will write
 * @code
 * auto factorial = ltl::fix {
 *  [](auto f, auto x) -> int {
 *      return x ? x * f(x - 1) : 1;
 *  }
 * };
 * fix(5); // returns 5 * 4 * 3 * 2 * 1 = 120
 *
 * @endcode
 * The trailing returns is necessary because without it, the compiler cannot deduce the return type.
 */
struct fix : F {
    /// \cond
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
    /// \endcond
};

/// \cond
template <typename F>
fix(F)->fix<F>;
/// \endcond

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

template <typename F, typename... Fs>
/**
 * @brief Compose the function in the left to right order.
 *
 * Contrary to maths where g o f means g(f(x)), compose(g, f) will do f(g(x))
 * It allows to write in a more expressive and a more natural way some operations where ones would write lambda instead.
 *
 * Let's say we have a Person with a name. If you want to get the size of the name of a person, you can do
 * it in several ways. Here are three equivalents, one using compose, one without, and one trivial.
 *
 * @code
 * struct Person {
 *     std::string name;
 * };
 *
 * Person p;
 * auto size1 = p.name.size(); // Simple because we don't need a predicate here
 * auto size2 = [](const auto &person) {return person.name.size(); }(p);
 * auto size3 = ltl::compose(&Person::name, &std::string::size)(p);
 * @endcode
 *
 * In real-world code, you will probably never use compose directly, but indirectly within algorithms, or range
 * features.
 * @param f
 * @param fs
 */
constexpr auto compose(F f, Fs... fs) {
    if constexpr (sizeof...(Fs) == 0) {
        return f;
    } else {
        return [f, fs...](auto &&... xs) -> decltype(auto) { //
            return ltl::fast_invoke(compose(fs...), ltl::fast_invoke(f, FWD(xs)...));
        };
    }
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

/// @}
///
/// \defgroup Predicate The predicate group
/// @{

template <typename... Fs>
/**
 * @brief This function negates its input
 *
 * If one wants to find someone in a vector of person with a non empty name, it may use this function
 *
 * @code
 *  struct Person {
 *      std::string name;
 *  };
 *  std::vector<Person> persons;
 *  auto person = ltl::find_if(persons, &Person::name, ltl::not_(&std::string::empty));
 * @endcode
 * @param fs
 */
constexpr auto not_(Fs... fs) {
    return [f = compose(std::move(fs)...)](auto &&... xs) { return !ltl::fast_invoke(f, FWD(xs)...); };
}

template <typename... Fs>
/**
 * @brief This function transforms a list of predicates into fs[0] || fs[1] || ...
 *
 * If one wants to find someone in a vector of person named John or Bill, it may write
 *
 * @code
 *  struct Person {
 *      std::string name;
 *  };
 *  std::vector<Person> persons;
 *  auto person = ltl::find_if(persons, &Person::name, ltl::or_(ltl::equal_to("Bill"), ltl::equal_to("John")));
 * @endcode
 * @param fs
 */
constexpr auto or_(Fs... fs) {
    return [fs...](auto &&... xs) { return (false_v || ... || (fs(FWD(xs)...))); };
}

template <typename... Fs>
/**
 * @brief This function transforms a list of predicates into fs[0] && fs[1] && ...
 *
 * If one wants to find a value between 18 and 25, it may write
 *
 * @code
 *  std::vector<int> values;
 *  auto value = ltl::find_if(values, ltl::and_(ltl::less_than(25), ltl::greater_than(18)));
 * @endcode
 * @param fs
 */
constexpr auto and_(Fs... fs) {
    return [fs...](auto &&... xs) { return (true_v && ... && (fs(FWD(xs)...))); };
}

template <typename F>
struct Predicate {
    F f;

    template <typename... Args>
    constexpr auto operator()(Args &&... args) const noexcept {
        return ltl::fast_invoke(f, FWD(args)...);
    }

    template <typename... Args>
    constexpr auto operator()(Args &&... args) noexcept {
        return ltl::fast_invoke(f, FWD(args)...);
    }
};

template <typename F>
Predicate(F)->Predicate<F>;

template <typename F1, typename F2>
constexpr auto operator||(Predicate<F1> p1, Predicate<F2> p2) {
    return Predicate{or_(std::move(p1), std::move(p2))};
}

template <typename F1, typename F2>
constexpr auto operator&&(Predicate<F1> p1, Predicate<F2> p2) {
    return Predicate{and_(std::move(p1), std::move(p2))};
}

/// \cond

template <typename F>
struct Does {
    F f;

#define OP(op)                                                                                                         \
    template <typename T>                                                                                              \
    constexpr auto operator op(T t) const noexcept {                                                                   \
        return Predicate{[this, t = std::move(t)](auto &&x) { return ltl::fast_invoke(f, FWD(x)) op t; }};             \
    }

    OP(==)
    OP(!=)
    OP(<)
    OP(<=)
    OP(>)
    OP(>=)
#undef OP

    constexpr auto operator!() const noexcept {
        return [this](auto &&x) { return !ltl::fast_invoke(f, FWD(x)); };
    }
};

template <typename F>
Does(F)->Does<F>;

/// \endcond

template <typename... Fs>
/**
 * @brief does - This function is meant to be used as a predicate builder
 *
 * When one wants to test something, for example with a find_if, it may use the does function to build a suitable
 * predicate
 *
 * @code
 *  struct Person {
 *      std::string name;
 *  };
 *
 *  std::vector<Person> persons;
 *  auto bill = ltl::find_if(persons, ltl::does(&Person::name) == "Bill");
 *  auto shortNamePerson = ltl::find_if(persons, ltl::does(&Person::name, &std::string::size) < 4);
 *
 * @endcode
 * @param fs
 */
constexpr auto does(Fs... fs) {
    auto f = compose(std::move(fs)...);
    return Does{f};
}

template <typename T>
/**
 * @brief less_than
 *
 * @param t
 */
constexpr auto less_than(T t) {
    return Predicate{[t = std::move(t)](auto x) { return x < t; }};
}

template <typename T>
/**
 * @brief less_than_equal
 *
 * @param t
 */
constexpr auto less_than_equal(T t) {
    return Predicate{[t = std::move(t)](const auto &x) { return x <= t; }};
}

template <typename T>
constexpr auto greater_than(T t) {
    return Predicate{[t = std::move(t)](const auto &x) { return x > t; }};
}

template <typename T>
constexpr auto greater_than_equal(T t) {
    return Predicate{[t = std::move(t)](const auto &x) { return x >= t; }};
}

template <typename T>
constexpr auto equal_to(T t) {
    return Predicate{[t = std::move(t)](const auto &x) { return x == t; }};
}

template <typename T>
constexpr auto not_equal_to(T t) {
    return Predicate{[t = std::move(t)](const auto &x) { return x != t; }};
}

template <typename... Fs>
constexpr auto byAscending(Fs... fs) {
    return [f = compose(std::move(fs)...)](const auto &x, const auto &y) noexcept { //
        return ltl::fast_invoke(f, x) < ltl::fast_invoke(f, y);
    };
};

template <typename... Fs>
constexpr auto byDescending(Fs... fs) {
    return [f = compose(std::move(fs)...)](const auto &x, const auto &y) noexcept { //
        return ltl::fast_invoke(f, x) > ltl::fast_invoke(f, y);
    };
};

/// @}

} // namespace ltl
