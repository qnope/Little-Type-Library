/**
 * @file fast.h
 */
#pragma once

#include "ltl/traits.h"
#include <array>
#include <optional>
#include <type_traits>

namespace ltl {
namespace fast {

/**
 *\defgroup Fast Fast metaprogramming types
 *@{
 */

/// \cond

namespace details {

template <std::size_t N>
constexpr std::optional<int> get_index(std::array<bool, N> array, int first) {
    for (std::size_t i = first; i < N; ++i)
        if (array[i])
            return int(i);
    return std::nullopt;
}

template <std::size_t N>
constexpr std::size_t count_ones(std::array<bool, N> array) {
    std::size_t r = 0;
    for (auto x : array) {
        if (x)
            ++r;
    }
    return r;
}

template <std::size_t NewN, std::size_t N>
constexpr auto convert_ones_to_indices(std::array<bool, N> array) {
    std::array<int, NewN> result{};
    int j = 0;
    for (std::size_t i = 0; i < N; ++i) {
        if (array[i]) {
            result[j++] = int(i);
        }
    }
    return result;
}
} // namespace details

/// \endcond

template <typename... Ts>
/**
 * @brief The type_list struct
 *
 * You can define a type_list with `type_list<T1, T2, T3...>`
 * You may use such list for template meta programming
 */
struct type_list {};

template <std::size_t Index, typename List, typename X = std::make_index_sequence<Index>>
struct at;

template <std::size_t Index, template <typename... Ts> typename List, typename... Ts, std::size_t... Is>
/**
 * @brief at. You may use at to get a type from a list of type, like a std::variant, type_list...
 */
struct at<Index, List<Ts...>, std::index_sequence<Is...>> {
    template <typename T>
    static constexpr T impl_f(decltype(Is, (const void *)0)..., T *, ...);

    using type = decltype(impl_f(std::declval<Ts *>()...));
};

template <std::size_t Index, typename List>
/**
 * @brief at
 *
 * @code
 * using List = type_list<int, double, char>;
 * static_assert(std::is_same_v<double, at_t<1, List>>);
 * @endcode
 */
using at_t = typename at<Index, List>::type;

template <typename T, typename List, int First = 0>
class find;

template <typename T, template <typename... Ts> typename List, typename... Ts, int First>
/**
 * @brief find - Get the index of T in List
 *
 * Returns an optional<int> of the index of T in the List
 */
class find<T, List<Ts...>, First> {
    static constexpr std::array<bool, sizeof...(Ts)> array = {std::is_same<T, Ts>::value...};

  public:
    static constexpr auto value = details::get_index(array, First);
};

template <typename T, typename List, int First = 0>
/**
 * @brief find_v
 *
 * @code
 * using List = type_list<int, double, char>;
 * constexpr auto x = find_v<char, List>;
 * constexpr auto y = find_v<long, List>;
 * static_assert(*x == 2);
 * static_assert(y.has_value() == false);
 * @endcode
 */
constexpr std::optional<int> find_v = find<T, List, First>::value;

template <template <typename T> typename P, typename List, int First = 0>
class find_if;

template <template <typename T> typename P, template <typename... Ts> typename List, typename... Ts, int First>
/**
 * @brief find_if Retrieve the index of the first element in List that satisfies Predicate P
 */
class find_if<P, List<Ts...>, First> {
    static constexpr std::array<bool, sizeof...(Ts)> array = {P<Ts>::value...};

  public:
    static constexpr auto value = details::get_index(array, First);
};

template <template <typename T> typename P, typename List, int First = 0>
/**
 * @brief find_if_v
 *
 * @code
 * using List = type_list<int, double, double*>;
 * constexpr auto x = find_if_v<std::is_pointer, List>;
 * static_assert(x == 2);
 * @endcode
 */
constexpr std::optional<int> find_if_v = find_if<P, List, First>::value;

template <typename T, typename List>
struct count;

template <typename T, template <typename... Ts> typename List, typename... Ts>
/**
 * @brief The count struct - Count the number of T in List
 */
struct count<T, List<Ts...>> {
    static constexpr int value = (0 + ... + (int(std::is_same<T, Ts>::value)));
};

template <typename T, typename List>
/**
 * @brief count_v
 *
 * @code
 * using List = type_list<int, double, double>;
 * static_assert(count_v<double, List> == 2);
 * @endcode
 */
constexpr int count_v = count<T, List>::value;

template <template <typename T> typename P, typename List>
struct count_if;

template <template <typename T> typename P, template <typename... Ts> typename List, typename... Ts>
/**
 * @brief The count_if struct - Count the number of element that satisfies P in List
 */
struct count_if<P, List<Ts...>> {
    static constexpr int value = (0 + ... + (int(P<Ts>::value)));
};

template <template <typename T> typename P, typename List>
/**
 * @brief count_if_v
 *
 * @code
 * using List = type_list<int, double, double*>;
 * static_assert(count_if<std::is_pointer, List> == 1);
 * @endcode
 */
constexpr int count_if_v = count_if<P, List>::value;

template <typename List>
struct is_unique;

template <template <typename... Ts> typename List, typename... Ts>
/**
 * @brief The is_unique struct - To know if the List does not contain duplicate
 */
struct is_unique<List<Ts...>> {
    static constexpr bool value = (true && ... && (count<Ts, List<Ts...>>::value == 1));
};

template <typename List>
/**
 * @brief is_unique_v
 *
 * @code
 * using List = type_list<int, double, char>;
 * static_assert(is_unique_v<List>);
 * @endcode
 */
constexpr bool is_unique_v = is_unique<List>::value;

template <typename List, template <typename...> typename X>
struct rename;

template <template <typename...> typename List, template <typename...> typename X, typename... Ts>
/**
 * @brief The rename struct - Transform a List to another one.
 */
struct rename<List<Ts...>, X> {
    using type = X<Ts...>;
};

template <typename List, template <typename...> typename X>
/**
 * @brief rename_t
 *
 * @code
 * using List = type_list<int, double, char>;
 * using variant = rename_t<List, std::variant>;
 * @endcode
 */
using rename_t = typename rename<List, X>::type;

template <typename T, typename List>
struct contains;

template <typename T, template <typename...> typename List, typename... Ts>
/**
 * @brief The contains struct - To know if the List contains T
 */
struct contains<T, List<Ts...>> {
    static constexpr auto value = (... || std::is_same<T, Ts>::value);
};

template <typename T, typename List>
/**
 * @brief contains_v
 *
 * @code
 * using List = type_list<int, double, char>;
 * static_assert(contains_v<int, List>);
 * @endcode
 */
inline constexpr bool contains_v = contains<T, List>::value;

template <template <typename T> typename P, typename List>
struct contains_if;

template <template <typename T> typename P, template <typename...> typename List, typename... Ts>
/**
 * @brief The contains_if struct - To know if the List contains an element satisfying P
 */
struct contains_if<P, List<Ts...>> {
    static constexpr auto value = (... || P<Ts>::value);
};

template <template <typename T> typename P, typename List>
/**
 * @brief contains_if_v
 *
 * @code
 * using List = type_list<int, double, char*>;
 * static_assert(contains_if_v<std::is_pointer, List>);
 * @endcode
 */
inline constexpr bool contains_if_v = contains_if<P, List>::value;

template <typename List>
class unique;

template <template <typename...> typename List, typename... Xs>
/**
 * @brief The unique class - to remove duplicates
 */
class unique<List<Xs...>> {
    static constexpr auto total_size = sizeof...(Xs);

    template <typename Indexer>
    struct impl;

    template <int... Is>
    struct impl<std::integer_sequence<int, Is...>> {
        static constexpr std::array<bool, total_size> ones = {(find<Xs, List<Xs...>>::value == Is)...};
        static constexpr auto new_size = details::count_ones(ones);
        static constexpr auto indices = details::convert_ones_to_indices<new_size>(ones);
        using indexer = typename ::ltl::array_to_index_sequence<indices>::type;
    };

  public:
    using indexer = typename impl<std::make_integer_sequence<int, total_size>>::indexer;
};

template <typename List>
/**
 * @brief unique_indexer
 *
 * @code
 *  constexpr ltl::tuple_t<int, double, int> x {5, 3.0, 8};
 *  using indexer = unique_indexer<decltype(x)>;
 *  static_assert(x.extract(indexer{}) == ltl::tuple_t{5, 3.0});
 * @endcode
 */
using unique_indexer = typename unique<List>::indexer;

template <typename List, template <typename> typename Predicate>
class filter;

template <template <typename...> typename List, typename... Xs, template <typename> typename Predicate>
/**
 * @brief The filter class - to filter a type list
 */
class filter<List<Xs...>, Predicate> {
    static constexpr ::std::array<bool, sizeof...(Xs)> ones = {Predicate<Xs>::value...};
    static constexpr auto newN = (int(Predicate<Xs>::value) + ... + 0);
    static constexpr auto indices = details::convert_ones_to_indices<newN>(ones);

  public:
    using indexer = typename ::ltl::array_to_index_sequence<indices>::type;
};

template <typename List, template <typename> typename Predicate>
/**
 * @brief filter_indexer
 *
 * @code
 *  constexpr ltl::tuple_t<int, double, int*> x {5, 3.0, nullptr};
 *  using indexer = filter_indexer<decltype(x), std::is_pointer>;
 *  static_assert(x.extract(indexer{}) == ltl::tuple_t<int*>{nullptr});
 * @endcode
 */
using filter_indexer = typename filter<List, Predicate>::indexer;

template <typename List, template <typename> typename Predicate>
struct all_of;
template <typename List, template <typename> typename Predicate>
struct any_of;
template <typename List, template <typename> typename Predicate>
struct none_of;

template <template <typename...> typename List, typename... Xs, template <typename> typename Predicate>
/**
 * @brief The all_of struct - Check if all elements in list satisfy Predicate
 */
struct all_of<List<Xs...>, Predicate> {
    static constexpr auto value = (Predicate<Xs>::value && ...);
};

template <template <typename...> typename List, typename... Xs, template <typename> typename Predicate>
/**
 * @brief The any_of struct - Check if any elements in list satisfy Predicate
 */
struct any_of<List<Xs...>, Predicate> {
    static constexpr auto value = (Predicate<Xs>::value || ...);
};

template <template <typename...> typename List, typename... Xs, template <typename> typename Predicate>
/**
 * @brief The none_of struct - Check if none elements in list satisfy Predicate
 */
struct none_of<List<Xs...>, Predicate> {
    static constexpr auto value = !(Predicate<Xs>::value || ...);
};

template <typename List, template <typename> typename Predicate>
/**
 * @brief all_of_v
 *
 * @code
 *  static_assert(all_of_v<type_list<int*, double*>, std::is_pointer>);
 * @endcode
 */
inline constexpr auto all_of_v = all_of<List, Predicate>::value;

template <typename List, template <typename> typename Predicate>
/**
 * @brief any_of_v
 *
 * @code
 *  static_assert(any_of_v<type_list<int, double*>, std::is_pointer>);
 * @endcode
 */
inline constexpr auto any_of_v = any_of<List, Predicate>::value;

template <typename List, template <typename> typename Predicate>
/**
 * @brief none_of_v
 *
 * @code
 *  static_assert(none_of_v<type_list<int, double>, std::is_pointer>);
 * @endcode
 */
inline constexpr auto none_of_v = none_of<List, Predicate>::value;

template <template <typename...> typename F, typename... Xs>
/**
 * @brief The bind1st struct - Transform a n-ary function into a unary function by binding Xs to its first arguments
 *
 *
 */
struct bind1st {
    template <typename Y>
    using apply = F<Xs..., Y>;
};

template <template <typename...> typename F, typename... Ys>
/**
 * @brief The bind2nd struct - Transform a n-ary function into a unary function by binding Ys to its last arguments
 */
struct bind2nd {
    template <typename X>
    using apply = F<X, Ys...>;
};

template <typename F>
/**
 * @brief The function_to_metafunction struct - transform a runtime function into a compile time one
 */
struct function_to_metafunction {
    template <typename X>
    using apply = decltype(std::declval<F>()(std::declval<X>()));
};

template <typename List>
struct qualified_type_list;

template <template <typename...> typename List, typename... Types>
/**
 * @brief The qualified_type_list struct - Transform a cvref qualified list into a List of cvref element
 */
struct qualified_type_list<List<Types...>> {
    using type = type_list<Types...>;
};

template <template <typename...> typename List, typename... Types>
/**
 * @brief The qualified_type_list struct - Transform a cvref qualified list into a List of cvref element
 */
struct qualified_type_list<const List<Types...>> {
    using type = type_list<std::add_const_t<Types>...>;
};

template <template <typename...> typename List, typename... Types>
/**
 * @brief The qualified_type_list struct - Transform a cvref qualified list into a List of cvref element
 */
struct qualified_type_list<List<Types...> &> {
    using type = type_list<std::add_lvalue_reference_t<Types>...>;
};

template <template <typename...> typename List, typename... Types>
/**
 * @brief The qualified_type_list struct - Transform a cvref qualified list into a List of cvref element
 */
struct qualified_type_list<const List<Types...> &> {
    using type = type_list<std::add_lvalue_reference_t<std::add_const_t<Types>>...>;
};

template <typename List>
/**
 * @brief qualified_type_list_t - Transform a cvref qualified list into a List of cvref element
 */
using qualified_type_list_t = qualified_type_list<List>;

template <typename List, template <typename> typename F>
struct apply;

template <template <typename...> typename List, template <typename> typename F, typename... Types>
/**
 * @brief The apply struct - Apply the F meta function to each elements of List
 */
struct apply<List<Types...>, F> {
    using type = type_list<F<Types>...>;
};

template <typename List, template <typename> typename F>
/**
 * @brief apply_t
 *
 * @code
 *  using List = type_list<int, double, char>;
 *  using ListPointer = apply<List, std::add_pointer>;
 *  static_assert(is_same_v<ListPointer, type_list<int*, double*, char*>);
 * @endcode
 */
using apply_t = typename apply<List, F>::type;

template <bool>
/**
 * @brief The conditional struct - A fast compiling conditional trait
 */
struct conditional {
    template <typename T, typename F>
    using apply = F;
};

template <>
struct conditional<true> {
    template <typename T, typename F>
    using apply = T;
};

template <bool cond, typename T, typename F>
/**
 * @brief conditional_t - A fast compiling `std::conditional`
 */
using conditional_t = typename conditional<cond>::template apply<T, F>;

template <typename T>
using remove_rvalue_reference_t =
    typename conditional<std::is_lvalue_reference_v<T>>::template apply<T, ltl::remove_cvref_t<T>>;

/**
 * @}
 */

} // namespace fast

} // namespace ltl
