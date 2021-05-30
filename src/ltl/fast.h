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

template <typename... Ts>
struct type_list {};

template <std::size_t Index, typename X = std::make_index_sequence<Index>>
struct at;

template <std::size_t Index, std::size_t... Xs>
struct at<Index, std::index_sequence<Xs...>> {
    template <typename T>
    static constexpr T impl_f(decltype(Xs, (const void *)0)..., T *, ...);

    template <typename List>
    struct impl;

    template <template <typename...> typename List, typename... Ts>
    struct impl<List<Ts...>> {
        using type = decltype(impl_f(std::declval<Ts *>()...));
    };

    template <typename List>
    using type = typename impl<List>::type;
};

template <typename T, typename List, int First = 0>
class find;

template <typename T, template <typename... Ts> typename List, typename... Ts, int First>
class find<T, List<Ts...>, First> {
    static constexpr std::array<bool, sizeof...(Ts)> array = {std::is_same<T, Ts>::value...};

  public:
    static constexpr auto value = details::get_index(array, First);
};

template <typename T, typename List, int First = 0>
constexpr std::optional<int> find_v = find<T, List, First>::value;

template <template <typename T> typename P, typename List, int First = 0>
class find_if;

template <template <typename T> typename P, template <typename... Ts> typename List, typename... Ts, int First>
class find_if<P, List<Ts...>, First> {
    static constexpr std::array<bool, sizeof...(Ts)> array = {P<Ts>::value...};

  public:
    static constexpr auto value = details::get_index(array, First);
};

template <template <typename T> typename P, typename List, int First = 0>
constexpr std::optional<int> find_if_v = find_if<P, List, First>::value;

template <typename T, typename List>
struct count;

template <typename T, template <typename... Ts> typename List, typename... Ts>
struct count<T, List<Ts...>> {
    static constexpr int value = (0 + ... + (int(std::is_same<T, Ts>::value)));
};

template <typename T, typename List>
constexpr int count_v = count<T, List>::value;

template <template <typename T> typename P, typename List>
struct count_if;

template <template <typename T> typename P, template <typename... Ts> typename List, typename... Ts>
struct count_if<P, List<Ts...>> {
    static constexpr int value = (0 + ... + (int(P<Ts>::value)));
};

template <template <typename T> typename P, typename List>
constexpr int count_if_v = count_if<P, List>::value;

template <typename List>
struct is_unique;

template <template <typename... Ts> typename List, typename... Ts>
struct is_unique<List<Ts...>> {
    static constexpr bool value = (true && ... && (count<Ts, List<Ts...>>::value == 1));
};

template <typename List>
constexpr bool is_unique_v = is_unique<List>::value;

template <typename List, template <typename...> typename X>
struct rename;

template <template <typename...> typename List, template <typename...> typename X, typename... Ts>
struct rename<List<Ts...>, X> {
    using type = X<Ts...>;
};

template <typename List, template <typename...> typename X>
using rename_t = typename rename<List, X>::type;

template <typename T, typename List>
struct contains;

template <typename T, template <typename...> typename List, typename... Ts>
struct contains<T, List<Ts...>> {
    static constexpr auto value = (... || std::is_same<T, Ts>::value);
};

template <typename T, typename List>
inline constexpr bool contains_v = contains<T, List>::value;

template <template <typename T> typename P, typename List>
struct contains_if;

template <template <typename T> typename P, template <typename...> typename List, typename... Ts>
struct contains_if<P, List<Ts...>> {
    static constexpr auto value = (... || P<Ts>::value);
};

template <template <typename T> typename P, typename List>
inline constexpr bool contains_if_v = contains_if<P, List>::value;

template <typename List>
class unique;

template <template <typename...> typename List, typename... Xs>
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
using unique_indexer = typename unique<List>::indexer;

template <typename List, template <typename> typename Predicate>
class filter;

template <template <typename...> typename List, typename... Xs, template <typename> typename Predicate>
class filter<List<Xs...>, Predicate> {
    static constexpr ::std::array<bool, sizeof...(Xs)> ones = {Predicate<Xs>::value...};
    static constexpr auto newN = (int(Predicate<Xs>::value) + ... + 0);
    static constexpr auto indices = details::convert_ones_to_indices<newN>(ones);

  public:
    using indexer = typename ::ltl::array_to_index_sequence<indices>::type;
};

template <typename List, template <typename> typename Predicate>
using filter_indexer = typename filter<List, Predicate>::indexer;

template <typename List, template <typename> typename Predicate>
struct all_of;
template <typename List, template <typename> typename Predicate>
struct any_of;
template <typename List, template <typename> typename Predicate>
struct none_of;

template <template <typename...> typename List, typename... Xs, template <typename> typename Predicate>
struct all_of<List<Xs...>, Predicate> {
    static constexpr auto value = (Predicate<Xs>::value && ...);
};
template <template <typename...> typename List, typename... Xs, template <typename> typename Predicate>
struct any_of<List<Xs...>, Predicate> {
    static constexpr auto value = (Predicate<Xs>::value || ...);
};
template <template <typename...> typename List, typename... Xs, template <typename> typename Predicate>
struct none_of<List<Xs...>, Predicate> {
    static constexpr auto value = !(Predicate<Xs>::value || ...);
};

template <typename List, template <typename> typename Predicate>
inline constexpr auto all_of_v = all_of<List, Predicate>::value;

template <typename List, template <typename> typename Predicate>
inline constexpr auto any_of_v = any_of<List, Predicate>::value;

template <typename List, template <typename> typename Predicate>
inline constexpr auto none_of_v = none_of<List, Predicate>::value;

template <template <typename...> typename F, typename... Xs>
struct bind1st {
    template <typename Y>
    using apply = F<Xs..., Y>;
};

template <template <typename...> typename F, typename... Ys>
struct bind2nd {
    template <typename X>
    using apply = F<X, Ys...>;
};

template <typename F>
struct function_to_metafunction {
    template <typename X>
    using apply = decltype(std::declval<F>()(std::declval<X>()));
};

template <typename List>
struct qualified_type_list;

template <template <typename...> typename List, typename... Types>
struct qualified_type_list<List<Types...>> {
    using type = type_list<Types...>;
};

template <template <typename...> typename List, typename... Types>
struct qualified_type_list<const List<Types...>> {
    using type = type_list<std::add_const_t<Types>...>;
};

template <template <typename...> typename List, typename... Types>
struct qualified_type_list<List<Types...> &> {
    using type = type_list<std::add_lvalue_reference_t<Types>...>;
};

template <template <typename...> typename List, typename... Types>
struct qualified_type_list<const List<Types...> &> {
    using type = type_list<std::add_lvalue_reference_t<std::add_const_t<Types>>...>;
};

template <typename List>
using qualified_type_list_t = qualified_type_list<List>;

template <typename List, template <typename> typename F>
struct apply;

template <template <typename...> typename List, template <typename> typename F, typename... Types>
struct apply<List<Types...>, F> {
    using type = type_list<F<Types>...>;
};

template <typename List, template <typename> typename F>
using apply_t = typename apply<List, F>::type;

/**
 * @}
 */

} // namespace fast

} // namespace ltl
