/**
 * @file actions.h
 */
#pragma once

#include "ltl/algos.h"
#include "ltl/concept.h"
#include "ltl/functional.h"

#include "Taker.h"

namespace ltl {

/// The actions namespace
namespace actions {

/**
 * \defgroup Actions The actions group
 * @{
 */

using std::begin;
using std::end;

/// \cond

struct AbstractAction {};
struct AbstractModifyingAction : AbstractAction {};

template <typename T>
constexpr bool IsAction = std::is_base_of_v<AbstractAction, ltl::remove_cvref_t<T>>;

template <typename T>
constexpr bool IsModifyingAction = std::is_base_of_v<AbstractModifyingAction, ltl::remove_cvref_t<T>>;

struct Sort : AbstractModifyingAction {};

template <typename F>
struct SortBy : AbstractModifyingAction {
    SortBy(F &&f) : f{std::move(f)} {}
    F f;
};

template <typename D>
struct JoinWith : AbstractAction {
    JoinWith(D &&d) : d{static_cast<D &&>(d)} {}
    D d;
};

template <typename T, typename F>
struct Accumulate : AbstractAction {
    Accumulate(T &&init, F &&f) : init{FWD(init)}, f{static_cast<F &&>(f)} {}
    T init;
    F f;
};

struct Sum : AbstractAction {};

/// \endcond
/**
 * @brief sort - action to sort an array
 *
 * @code
 *  std::vector<int> values;
 *  values |= ltl::actions::sort;
 * @endcode
 */
inline constexpr Sort sort{};

template <typename F>
/**
 * @brief sort_by - Sort according to the given comparator function
 *
 * This function will generally not be used and users will prefer to use directly `ltl::sort_by_ascending` or
 * `ltl::sort_by_descending`
 * @param f
 */
constexpr auto sort_by(F f) {
    return SortBy<F>{std::move(f)};
}

template <typename... Fs>
/**
 * @brief sort_by_ascending - action to sort by ascending order
 *
 * @code
 *  struct Person {
 *      std::string name;
 *  };
 *
 *  std::vector<Person> persons;
 *  std::vector<Person> sorted_persons = persons | ltl::actions::sort_by_ascending(&Person::name);
 * @endcode
 * @param fs
 */
constexpr auto sort_by_ascending(Fs... fs) {
    return sort_by(ltl::byAscending(std::move(fs)...));
}

template <typename... Fs>
/**
 * @brief sort_by_descending - action to sort by descending order
 *
 * @code
 *  struct Person {
 *      std::string name;
 *  };
 *
 *  std::vector<Person> persons;
 *  std::vector<Person> sorted_persons = persons | ltl::actions::sort_by_descending(&Person::name);
 * @endcode
 * @param fs
 */
constexpr auto sort_by_descending(Fs... fs) {
    return sort_by(ltl::byDescending(std::move(fs)...));
}

struct Unique : AbstractModifyingAction {};
struct Reverse : AbstractModifyingAction {};

/**
 * @brief unique - action to remove adjacent duplicates
 *
 * @code
 *  std::vector<int> values;
 *  values |= ltl::actions::sort | ltl::actions::unique;
 * @endcode
 */
inline constexpr Unique unique{};

/**
 * @brief reverse - action to reverse the order of an array
 *
 * @code
 *  std::vector<int> values;
 *  values |= ltl::actions::reverse;
 * @endcode
 */
inline constexpr Reverse reverse{};

/// \cond

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

template <typename T>
struct FindNullable : AbstractAction {
    FindNullable(T &&t) : elem{static_cast<T &&>(t)} {}
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

template <typename F>
struct FindIfNullable : AbstractAction {
    FindIfNullable(F &&f) : f{static_cast<F &&>(f)} {}
    F f;
};

/// \endcond

template <typename T>
/**
 * @brief find - return an iterator on the first element equal to e
 *
 * returns end if no element is found
 *
 * @code
 *  std::vector<int> array;
 *  auto it = array | ltl::actions::find(e)
 * @endcode
 * @param e
 */
constexpr auto find(T &&e) {
    return Find<T>{FWD(e)};
}

template <typename T>
/**
 * @brief find_value - return an optional on the first element equal to e
 *
 * returns nullopt if no element is found
 *
 * @code
 *  std::vector<int> array;
 *  auto value = array | ltl::actions::find_value(e)
 * @endcode
 * @param e
 */
constexpr auto find_value(T &&e) {
    return FindValue<T>{FWD(e)};
}

template <typename T>
/**
 * @brief find_ptr - return a pointer on the first element equal to e
 *
 * return nullptr if no element is found
 * @code
 *  std::vector<int> array;
 *  auto it = array | ltl::actions::find_value(e)
 * @endcode
 * @param e
 */
constexpr auto find_ptr(T &&e) {
    return FindPtr<T>{FWD(e)};
}

template <typename T>
/**
 * @brief find_nullable - return a first nullable element equal to e
 *
 * return nullptr if no element is found
 * @code
 *  std::vector<std::optional<int>> array;
 *  auto it = array | ltl::actions::find_nullable(e)
 * @endcode
 * @param e
 */
constexpr auto find_nullable(T &&e) {
    return FindNullable<T>{FWD(e)};
}

template <typename... Fs>
/**
 * @brief find_if - return an iterator on the first element satisfying the predicate
 *
 * returns end if no element is found
 *
 * The predicate is given as a composition of a function. This composition must go from it::value_type to bool
 *
 * @code
 *  struct Person {
 *      std::string name;
 *  };
 *  std::vector<Person> persons;
 *  auto billIt = array | ltl::actions::find_if(&Person::name, equal_to("Bill"));
 * @endcode
 * @param fs...
 */
constexpr auto find_if(Fs... fs) {
    return FindIf{compose(std::move(fs)...)};
}

template <typename... Fs>
/**
 * @brief find_if_value - return an optional on the first element satisfying the predicate
 *
 * returns nullopt if no element is found
 *
 * The predicate is given as a composition of a function. This composition must go from it::value_type to bool
 *
 * @code
 *  struct Person {
 *      std::string name;
 *  };
 *  std::vector<Person> persons;
 *  auto billOpt = array | ltl::actions::find_if_value(&Person::name, equal_to("Bill"));
 * @endcode
 * @param fs...
 */
constexpr auto find_if_value(Fs... fs) {
    return FindIfValue{compose(std::move(fs)...)};
}

template <typename... Fs>
/**
 * @brief find_if_ptr - return a pointer on the first element satisfying the predicate
 *
 * returns nullptr if no element is found
 *
 * The predicate is given as a composition of a function. This composition must go from it::value_type to bool
 *
 * @code
 *  struct Person {
 *      std::string name;
 *  };
 *  std::vector<Person> persons;
 *  auto billPtr = array | ltl::actions::find_if_ptr(&Person::name, equal_to("Bill"));
 * @endcode
 * @param fs...
 */
constexpr auto find_if_ptr(Fs... fs) {
    return FindIfPtr{compose(std::move(fs)...)};
}

template <typename... Fs>
/**
 * @brief find_if - return the first nullable element satisfying the predicate
 *
 * returns end if no element is found
 *
 * The predicate is given as a composition of a function. This composition must go from it::value_type to bool
 *
 * @code
 *  struct Person {
 *      std::string name;
 *  };
 *  std::vector<Person> persons;
 *  auto billIt = array | ltl::actions::find_if_nullable(&Person::name, equal_to("Bill"));
 * @endcode
 * @param fs...
 */
constexpr auto find_if_nullable(Fs... fs) {
    return FindIfNullable{compose(std::move(fs)...)};
}

template <typename D>
/**
 * @brief join_with - Join a list with a delimeter : Useful for strings
 *
 * @code
 *  std::vector<std::string> strings = {"My", "name", "is", "John"};
 *
 *  // string = "My name is John";
 *  auto string = strings | ltl::actions::join_with(' ');
 * @endcode
 * @param d
 */
constexpr auto join_with(D &&d) {
    return JoinWith<D>{FWD(d)};
}

template <typename T, typename F = std::plus<>>
/**
 * @brief accumulate - Perform a left fold
 *
 * @code
 *  std::vector<int> numbers;
 *  auto sum = numbers | ltl::actions::accumulate(0);
 * @endcode
 */
constexpr auto accumulate(T &&init, F f = F{}) {
    return Accumulate<T, F>{FWD(init), std::move(f)};
}

/**
 * @brief sum - Perform a sum
 *
 * @code
 *  std::vector<int> numbers;
 *  auto sum = numbers | ltl::actions::sum;
 * @endcode
 */
constexpr Sum sum{};

/// \cond

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

template <typename C, typename T, requires_f(ltl::IsIterable<C>)>
auto operator|(C &c, FindNullable<T> e) {
    return ::ltl::find_nullable(c, e.elem);
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

template <typename C, typename F, requires_f(ltl::IsIterable<C>)>
auto operator|(C &c, FindIfNullable<F> e) {
    return ::ltl::find_if_nullable(c, e.f);
}

template <typename C, typename D, requires_f(ltl::IsIterable<C>)>
auto operator|(const C &c, JoinWith<D> d) -> ltl::remove_cvref_t<decltype(*c.begin())> {
    if (c.empty()) {
        return {};
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
    return ltl::accumulate(c, ltl::remove_cvref_t<decltype(*begin(c))>{});
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

/// \endcond

/// @}

} // namespace actions

} // namespace ltl
