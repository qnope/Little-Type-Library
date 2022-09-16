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
    constexpr auto operator()(Args &&...args) const
        -> decltype(std::declval<const F &>()(std::declval<const fix &>(), std::declval<Args>()...)) {
        return static_cast<const F &>(*this)(*this, FWD(args)...);
    }

    template <typename... Args>
    constexpr auto operator()(Args &&...args)
        -> decltype(std::declval<F &>()(std::declval<fix &>(), std::declval<Args>()...)) {
        return static_cast<F &>(*this)(*this, FWD(args)...);
    }
    /// \endcond
};

/// \cond
template <typename F>
fix(F) -> fix<F>;
/// \endcond

template <typename F, typename... Args>
/**
 * @brief defer - This function is used to defer the call to f
 * @param f
 * @param xs
 */
constexpr auto defer(F f, Args... xs) {
    return [f = std::move(f), xs...](auto &&..._ys) -> decltype(auto) { return ltl::invoke(f, xs..., FWD(_ys)...); };
}

template <typename F, typename... Args>
/**
 * @brief curry - Currying is done to transform n ary function into unary ones
 *
 * Currying may be used to simplify some functions that are too complicates by saving the first args.
 */
constexpr decltype(auto) curry(F f, Args &&...args) {
    if constexpr (std::is_invocable_v<F, Args...>) {
        return ltl::invoke(f, FWD(args)...);
    } else {
        return defer(lift(curry), std::move(f), FWD(args)...);
    }
}

constexpr auto identity = [](auto &&t) -> fast::remove_rvalue_reference_t<decltype(FWD(t))> { return FWD(t); };
constexpr auto id_copy = [](auto x) { return std::move(x); };

/**
 * @brief compose - Version without argument
 *
 * For syntaxic sugar, compose() is the identity function
 */
constexpr auto compose() { return identity; }

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
        return [f, fs...](auto &&...xs) -> decltype(auto) { //
            return ltl::fast_invoke(compose(fs...), ltl::fast_invoke(f, FWD(xs)...));
        };
    }
}

template <typename F>
/**
 * @brief unzip - It is the opposite of zip. It will feed f with all elements of the tuple
 *
 * It may be useful when you have to deal with tuple_t.
 *
 * @code
 *  std::vector<ltl::tuple_t<int, double>> vector;
 *  // strings is a range of string in a form of : "int, double"
 *  auto strings = vector | map(unzip([](int i, double d) { return std::to_string(i) + "," + std::to_string(d);} );
 * @endcode
 * @param f
 */
constexpr auto unzip(F f) {
    return [f = std::move(f)](auto &&tuple) { return apply(f, FWD(tuple)); };
}

template <typename T, typename... Args>
/**
 * @brief construct - This function returns a builder of the type T
 *
 * This function may be used to convert a range of a type to another one
 * @code
 *  struct A {};
 *  struct B {
 *      B(A);
 *  };
 *  struct C {
 *      C(int, B);
 *  };
 *  std::vector<A> someAs;
 *  std::vector<B> someBs = someAs | map(construct<B>());
 *  std::vector<C> someCs = someBs | map(construct<C>(5));
 * @endcode
 *
 * Be careful, this function is lazy
 * @param args
 */
constexpr auto construct(Args... args) noexcept {
    return [args = tuple_t{std::move(args)...}](auto &&...ys) { //
        return apply(
            [&](auto &&...xs) { //
                return T{xs..., FWD(ys)...};
            },
            args);
    };
}

template <typename T, typename Tuple>
/**
 * @brief construct_with_tuple - This function builds the type T thanks to the given tuple
 *
 * Be careful, this function is eager
 * @param tuple
 */
constexpr auto construct_with_tuple(Tuple &&tuple) noexcept {
    return apply([](auto &&...xs) { return T{FWD(xs)...}; }, FWD(tuple));
}

template <typename T>
/**
 * @brief construct_with_tuple - This function returns a builder for the type T
 *
 * It may be used to convert a range of tuple into a range of a specific type
 *
 * @code
 *  struct Person {
 *      Person(int, std::string);
 *      int hp;
 *      std::string name;
 *  };
 *  std::vector<ltl::tuple_t<int, std::string>> values;
 *  std::vector<Person> persons = values | map(construct_with_tuple<Person>());
 * @endcode
 *
 * Be careful, this function is lazy
 */
constexpr auto construct_with_tuple() noexcept {
    return [](auto &&tuple) { //
        return apply(
            [](auto &&...xs) { //
                return T{FWD(xs)...};
            },
            FWD(tuple));
    };
}

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
 *
 * Generally, you don't want to use this function, use the Predicate structure operators !
 * @param fs
 */
constexpr auto not_(Fs... fs) {
    return [f = compose(std::move(fs)...)](auto &&...xs) { return !ltl::fast_invoke(f, FWD(xs)...); };
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
 *
 * Generally, you don't want to use this function, use the Predicate structure operators !
 * @param fs
 */
constexpr auto or_(Fs... fs) {
    return [fs...](auto &&...xs) { return (false_v || ... || (fs(FWD(xs)...))); };
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
 *
 * Generally, you don't want to use this function, use the Predicate structure operators !
 * @param fs
 */
constexpr auto and_(Fs... fs) {
    return [fs...](auto &&...xs) { return (true_v && ... && (invoke(fs, FWD(xs)...))); };
}

template <typename F>
/**
 * @brief The Predicate struct represents a predicate function to give to an algorithm
 *
 * Generally, such predicate will be created using less_than, greater_than, equal_to, not_equal_to, less_than_equal
 * or greater_than_equal.
 *
 * The purpose of this structure is to make it possible to write expressive combinaisons of predicates thanks to
 * `&&` and `||` operators:
 *
 * @code
 *  std::vector<int> values;
 *  auto value = ltl::find_if(values, equal_to(18) || less_than(10))
 * @endcode
 */
struct Predicate {
    /// \cond
    F f;

    template <typename... Args>
    constexpr auto operator()(Args &&...args) const noexcept {
        return ltl::fast_invoke(f, FWD(args)...);
    }

    template <typename... Args>
    constexpr auto operator()(Args &&...args) noexcept {
        return ltl::fast_invoke(f, FWD(args)...);
    }

    /// \endcond
};

/// \cond

template <typename F>
Predicate(F) -> Predicate<F>;

template <typename F1, typename F2>
constexpr auto operator||(Predicate<F1> p1, Predicate<F2> p2) {
    return Predicate{or_(std::move(p1), std::move(p2))};
}

template <typename F1, typename F2>
constexpr auto operator&&(Predicate<F1> p1, Predicate<F2> p2) {
    return Predicate{and_(std::move(p1), std::move(p2))};
}

template <typename F1>
constexpr auto operator!(Predicate<F1> p1) {
    return Predicate{not_(std::move(p1))};
}

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
Does(F) -> Does<F>;

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
 * @brief less_than- This function builds a predicate testing if a value is less than t
 *
 * @code
 *  std::vector<int> values;
 *  auto number = ltl::find_if(values, less_than(5));
 * @endcode
 *
 * @param t
 */
constexpr auto less_than(T t) {
    return Predicate{[t = std::move(t)](auto x) { return x < t; }};
}

template <typename T>
/**
 * @brief less_than_equal- This function builds a predicate testing if a value is less than or equal to t
 *
 * @code
 *  std::vector<int> values;
 *  auto number = ltl::find_if(values, less_than_equal(5));
 * @endcode
 *
 * @param t
 */
constexpr auto less_than_equal(T t) {
    return Predicate{[t = std::move(t)](const auto &x) { return x <= t; }};
}

template <typename T>
/**
 * @brief greater_than- This function builds a predicate testing if a value is greater than t
 *
 * @code
 *  std::vector<int> values;
 *  auto number = ltl::find_if(values, greater_than(5));
 * @endcode
 *
 * @param t
 */
constexpr auto greater_than(T t) {
    return Predicate{[t = std::move(t)](const auto &x) { return x > t; }};
}

template <typename T>
/**
 * @brief greater_than_equal- This function builds a predicate testing if a value is greater than or equal to t
 *
 * @code
 *  std::vector<int> values;
 *  auto number = ltl::find_if(values, greater_than_equal(5));
 * @endcode
 *
 * @param t
 */
constexpr auto greater_than_equal(T t) {
    return Predicate{[t = std::move(t)](const auto &x) { return x >= t; }};
}

template <typename T>
/**
 * @brief equal_to- This function builds a predicate testing if a value is equal to t
 *
 * @code
 *  std::vector<int> values;
 *  auto number = ltl::find_if(values, equal_to(5));
 * @endcode
 *
 * @param t
 */
constexpr auto equal_to(T t) {
    return Predicate{[t = std::move(t)](const auto &x) { return x == t; }};
}

template <typename T>
/**
 * @brief not_equal_to- This function builds a predicate testing if a value is not equal to t
 *
 * @code
 *  std::vector<int> values;
 *  auto number = ltl::find_if(values, not_equal_to(5));
 * @endcode
 *
 * @param t
 */
constexpr auto not_equal_to(T t) {
    return Predicate{[t = std::move(t)](const auto &x) { return x != t; }};
}

template <typename... Fs>
/**
 * @brief byAscending - This function builds a comparator that compares for ascending orders
 *
 * @code
 *  struct Person {
 *      std::string name;
 *  };
 *  std::vector<int> values;
 *  std::vector<Person> persons;
 *
 *  ltl::sort(values, byAscending()); // values are sorted in an ascending way
 *  ltl::sort(persons, byAscending(&Person::name)); // persons are sorted by ascending name
 * @endcode
 * @param fs
 */
constexpr auto byAscending(Fs... fs) {
    return [f = compose(std::move(fs)...)](const auto &x, const auto &y) noexcept { //
        return ltl::fast_invoke(f, x) < ltl::fast_invoke(f, y);
    };
}

template <typename... Fs>
/**
 * @brief byDescending - This function builds a comparator that compares for descending orders
 *
 * @code
 *  struct Person {
 *      std::string name;
 *  };
 *  std::vector<int> values;
 *  std::vector<Person> persons;
 *
 *  ltl::sort(values, byDescending()); // values are sorted in a descending way
 *  ltl::sort(persons, byDescending(&Person::name)); // persons are sorted by descending name
 * @endcode
 * @param fs
 */
constexpr auto byDescending(Fs... fs) {
    return [f = compose(std::move(fs)...)](const auto &x, const auto &y) noexcept { //
        return ltl::fast_invoke(f, x) > ltl::fast_invoke(f, y);
    };
}

/// @}

} // namespace ltl
