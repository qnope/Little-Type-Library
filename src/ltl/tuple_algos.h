#pragma once

#include "Tuple.h"
#include "functional.h"
#include "optional_type.h"

#define types_from(x) ::ltl::getQualifiedTypeList(type_from(x))

namespace ltl {
template <template <typename... Types> typename List, typename... Types>
constexpr auto getQualifiedTypeList(type_t<const List<Types...> &>) {
    return type_list_v<std::add_lvalue_reference_t<std::add_const_t<Types>>...>;
}

template <template <typename... Types> typename List, typename... Types>
constexpr auto getQualifiedTypeList(type_t<List<Types...> &>) {
    return type_list_v<std::add_lvalue_reference_t<Types>...>;
}

template <template <typename... Types> typename List, typename... Types>
constexpr auto getQualifiedTypeList(type_t<List<Types...> &&>) {
    return type_list_v<std::add_rvalue_reference_t<Types>...>;
}

template <typename F, typename Tuple>
constexpr auto transform_type(Tuple &&tuple, F &&f) {
    static_assert(IsTuple<Tuple>);
    auto build_tuple = [&f](auto &&... xs) {
        return ltl::tuple_t<decltype(static_cast<F &&>(f)(FWD(xs)))...>{static_cast<F &&>(f)(FWD(xs))...};
    };
    return FWD(tuple)(build_tuple);
}

template <typename Tuple, typename F>
constexpr auto accumulate_type(Tuple &&tuple, F &&f) {
    return FWD(tuple)(FWD(f));
}

template <typename Tuple>
constexpr auto accumulate_type(Tuple &&tuple) {
    auto accumulate = [](auto &&... xs) { return (... + FWD(xs)); };
    return accumulate_type(FWD(tuple), accumulate);
}

namespace details {
template <typename T>
constexpr bool is_type_related_list = ltl::IsTypeList<T> || ltl::IsNumberList<T> || ltl::IsBoolList<T>;

template <std::size_t N>
constexpr std::optional<int> get_index(std::array<bool, N> array, int first) {
    for (std::size_t i = first; i < N; ++i)
        if (array[i])
            return int(i);
    return std::nullopt;
}

template <std::size_t N>
constexpr std::size_t count(std::array<bool, N> array) {
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

////////////////////// Algorithm tuple
template <typename... Ts, typename T>
constexpr auto contains_type([[maybe_unused]] const tuple_t<Ts...> &tuple, [[maybe_unused]] type_t<T> type) {
    if constexpr (details::is_type_related_list<tuple_t<Ts...>>)
        return (false_v || ... || (Ts{} == type));
    else
        return contains_type(type_list_v<Ts...>, type);
}

template <typename... Ts, typename T>
constexpr auto count_type([[maybe_unused]] const tuple_t<Ts...> &tuple, [[maybe_unused]] T type) {
    if constexpr (details::is_type_related_list<tuple_t<Ts...>>)
        return (0_n + ... + bool_to_number(Ts{} == type));
    else
        return count_type(type_list_v<Ts...>, type);
}

template <typename... Ts, typename T, int N = 0>
constexpr auto find_type([[maybe_unused]] const tuple_t<Ts...> &tuple, [[maybe_unused]] T type,
                         number_t<N> first = {}) {
    if constexpr (details::is_type_related_list<tuple_t<Ts...>>) {
        constexpr std::array<bool, sizeof...(Ts)> array = {bool(Ts{} == T{})...};
        constexpr auto result = details::get_index(array, N);
        if constexpr (result)
            return optional_type<number_t<*result>>{};
        else
            return nullopt_type;
    }

    else
        return find_type(type_list_v<Ts...>, type, first);
}

template <typename... Ts, typename P, int N = 0>
constexpr auto find_if_type([[maybe_unused]] const tuple_t<Ts...> &tuple, [[maybe_unused]] P p,
                            number_t<N> first = {}) {
    if constexpr (details::is_type_related_list<tuple_t<Ts...>>) {
        constexpr std::array<bool, sizeof...(Ts)> array = {decltype(std::declval<P>()(Ts{}))::value...};
        constexpr auto result = details::get_index(array, N);
        if constexpr (result)
            return optional_type<number_t<*result>>{};
        else
            return nullopt_type;
    } else
        return find_if_type(type_list_v<Ts...>, p, first);
}

template <typename... Ts, typename P>
constexpr auto contains_if_type([[maybe_unused]] const tuple_t<Ts...> &tuple, [[maybe_unused]] P p) {
    if constexpr (details::is_type_related_list<tuple_t<Ts...>>)
        return (false_v || ... || p(Ts{}));
    else
        return contains_if_type(type_list_v<Ts...>, p);
}

template <typename... Ts, typename P>
constexpr auto count_if_type([[maybe_unused]] const tuple_t<Ts...> &tuple, [[maybe_unused]] P p) {
    if constexpr (details::is_type_related_list<tuple_t<Ts...>>)
        return (0_n + ... + bool_to_number(p(Ts{})));
    else
        return count_if_type(type_list_v<Ts...>, p);
}

template <typename... Ts, typename P>
constexpr auto all_of_type([[maybe_unused]] const tuple_t<Ts...> &tuple, [[maybe_unused]] P p) {
    if constexpr (details::is_type_related_list<tuple_t<Ts...>>)
        return (true_v && ... && (p(Ts{})));
    else
        return all_of_type(type_list_v<Ts...>, p);
}

template <typename... Ts, typename P>
constexpr auto any_of_type([[maybe_unused]] const tuple_t<Ts...> &tuple, [[maybe_unused]] P p) {
    if constexpr (details::is_type_related_list<tuple_t<Ts...>>)
        return (false_v || ... || (p(Ts{})));
    else
        return any_of_type(type_list_v<Ts...>, p);
}

template <typename... Ts, typename P>
constexpr auto none_of_type(const tuple_t<Ts...> &tuple, [[maybe_unused]] P p) {
    return !any_of_type(tuple, p);
}

namespace details {
template <typename Tuple, typename IndexSequence>
struct UniqueImpl {};

template <typename... Ts, std::size_t... Is>
struct UniqueImpl<tuple_t<Ts...>, std::index_sequence<Is...>> {
    static constexpr auto tuple = ltl::tuple_t<Ts...>{};
    static constexpr auto total_size = sizeof...(Is);
    static constexpr std::array<bool, total_size> ones = {
        decltype(*::ltl::find_type(tuple, Ts{}) == number_v<Is>)::value...};
    static constexpr auto new_size = count(ones);
    static constexpr auto indices = convert_ones_to_indices<new_size>(ones);
    static constexpr auto indexer = ::ltl::array_to_index_sequence_t<indices>{};
};
} // namespace details

template <typename... Ts>
constexpr auto unique_type(const tuple_t<Ts...> &tuple) {
    if constexpr (details::is_type_related_list<tuple_t<Ts...>>) { //
        return tuple.extract(details::UniqueImpl<tuple_t<Ts...>, std::make_index_sequence<sizeof...(Ts)>>::indexer);
    } else { //
        return unique_type(ltl::type_list_v<Ts...>);
    }
}

template <typename... Ts>
constexpr auto is_unique_type(const tuple_t<Ts...> &tuple) {
    if constexpr (details::is_type_related_list<tuple_t<Ts...>>) { //
        return (true_v && ... && (count_type(tuple, Ts{}) == 1_n));
    } else { //
        return is_unique_type(ltl::type_list_v<Ts...>);
    }
}

namespace details {
template <typename Predicate, typename... Ts>
struct FilterImpl {
    static constexpr ::std::array<bool, sizeof...(Ts)> ones = {decltype(::std::declval<Predicate>()(Ts{}))::value...};
    static constexpr auto newN = details::count(ones);
    static constexpr auto indices = details::convert_ones_to_indices<newN>(ones);
    static constexpr auto indexer = ::ltl::array_to_index_sequence_t<indices>{};
};
} // namespace details
template <typename... Ts, typename P>
constexpr auto filter_type(const tuple_t<Ts...> &tuple, [[maybe_unused]] P p) {
    if constexpr (details::is_type_related_list<tuple_t<Ts...>>) { //
        return tuple.extract(details::FilterImpl<P, Ts...>::indexer);
    } else {
        return filter_type(ltl::type_list_v<Ts...>, p);
    }
}

template <typename F, typename T, typename... Tuples, requires_f(ltl::IsTuple<T>)>
constexpr void zip_with(F &&f, T &&tuple, Tuples &&... tuples) {
    auto indexer = tuple.make_indexer();
    static_assert((IsTuple<Tuples> && ...), "All tuples must be tuples");
    typed_static_assert_msg((... && (tuples.length == indexer.length)), "All tuples must be of the same length");
    for_each(indexer,
             [&f, &tuple, &tuples...](auto indices) { static_cast<F &&>(f)(tuple[indices], tuples[indices]...); });
}

template <typename T, typename... Tuples, requires_f(ltl::IsTuple<T>)>
constexpr auto zip_type(T &&tuple, Tuples &&... tuples) {
    auto indexer = tuple.make_indexer();
    static_assert((IsTuple<Tuples> && ...), "All tuples must be tuples");
    typed_static_assert_msg((... && (tuples.length == indexer.length)), "All tuples must be of the same length");

    auto get_types_for_index = [&tuple, &tuples...](auto index) {
        return ltl::type_list_v<
            ltl::tuple_t<decltype_t(tuple.getTypes()[index]), decltype_t(tuples.getTypes()[index])...>>;
    };

    auto get_tuple_for_index = [&tuple, &tuples...](auto index) {
        return ltl::tuple_t<decltype_t(tuple.getTypes()[index]), decltype_t(tuples.getTypes()[index])...>{
            static_cast<T &&>(tuple)[index], static_cast<Tuples &&>(tuples)[index]...};
    };

    return indexer([get_types_for_index, get_tuple_for_index](auto... indices) {
        auto types = (... + (get_types_for_index(indices)));
        return types([get_tuple_for_index, indices...](auto... types) {
            return ltl::tuple_t<decltype_t(types)...>{get_tuple_for_index(indices)...};
        });
    });
}

template <typename F, typename T, requires_f(ltl::IsTuple<T>)>
constexpr void enumerate_with(F &&f, T &&tuple) {
    zip_with(FWD(f), tuple.make_indexer(), FWD(tuple));
}

template <typename T, requires_f(ltl::IsTuple<T>)>
constexpr auto enumerate_type(T &&tuple) {
    return zip_type(tuple.make_indexer(), FWD(tuple));
}

namespace detail {
template <typename F, typename T>
struct scanl_wrapper {
    scanl_wrapper(F f, T t) : f{f}, t{t} {}
    F f;
    T t;
};

template <typename F, typename T, typename N>
constexpr auto operator+(scanl_wrapper<F, T> wrapper, N &&newElement) {
    auto last = wrapper.t[wrapper.t.length - 1_n];
    auto newTuple = wrapper.t.push_back(wrapper.f(last, FWD(newElement)));
    return scanl_wrapper{wrapper.f, newTuple};
}
} // namespace detail

template <typename F, typename T, typename Tuple>
constexpr auto scanl([[maybe_unused]] F f, T init, Tuple &&tuple) {
    if_constexpr(tuple.isEmpty) { //
        return ltl::tuple_t<T>{std::move(init)};
    }
    else {
        return FWD(tuple)([f, init = std::move(init)](auto &&... xs) {
            return (detail::scanl_wrapper{f, ltl::tuple_t<T>{std::move(init)}} + ... + (FWD(xs))).t;
        });
    }
}

template <typename F, typename Tuple, requires_f(ltl::IsTuple<Tuple>)>
constexpr auto partial_sum(F f, Tuple &&tuple) {
    return scanl(std::move(f), 0_n, std::move(tuple)).pop_front();
}

template <template <typename...> typename T, typename List>
struct build_from_type_listImpl;

template <template <typename...> typename T, typename... Ts>
struct build_from_type_listImpl<T, type_list_t<Ts...>> {
    using type = T<Ts...>;
};

template <template <typename...> typename T, typename Ts>
using build_from_type_list = typename build_from_type_listImpl<T, std::decay_t<Ts>>::type;

} // namespace ltl
