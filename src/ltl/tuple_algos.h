#pragma once

#include "Tuple.h"
#include "functional.h"
#include "optional_type.h"

namespace ltl {

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

////////////////////// Algorithm tuple
template <typename List, typename T>
constexpr auto contains_type(const List &, type_t<T>) {
    return bool_v<fast::contains<T, List>::value>;
}

template <typename List, typename T>
constexpr auto count_type(const List &, type_t<T>) {
    return number_v<fast::count<T, List>::value>;
}

template <typename List, typename T, int N = 0>
constexpr auto find_type(const List &, type_t<T>, number_t<N> = {}) {
    constexpr auto result = fast::find<T, List, N>::value;
    if constexpr (result)
        return optional_type<number_t<*result>>{};
    else
        return nullopt_type;
}

template <typename List, typename P, int N = 0>
constexpr auto find_if_type(const List &, P p, number_t<N> = {}) {
    constexpr auto result = fast::find_if<fast::function_to_metafunction<P>::template apply, List, N>::value;
    if constexpr (result)
        return optional_type<number_t<*result>>{};
    else
        return nullopt_type;
}

template <typename List, typename P>
constexpr auto contains_if_type(const List &, P) {
    return bool_v<fast::contains_if<fast::function_to_metafunction<P>::template apply, List>::value>;
}

template <typename List, typename P>
constexpr auto count_if_type(const List &tuple, P) {
    return number_v<fast::count_if<fast::function_to_metafunction<P>::template apply, List>::value>;
}

template <typename List, typename P>
constexpr auto all_of_type(const List &, P) {
    return bool_v<fast::all_of_v<List, fast::function_to_metafunction<P>::template apply>>;
}

template <typename List, typename P>
constexpr auto any_of_type(const List &, P) {
    return bool_v<fast::any_of_v<List, fast::function_to_metafunction<P>::template apply>>;
}

template <typename List, typename P>
constexpr auto none_of_type(const List &, P) {
    return bool_v<fast::none_of_v<List, fast::function_to_metafunction<P>::template apply>>;
}

template <typename List>
constexpr auto unique_type(const List &tuple) {
    return tuple.extract(fast::unique_indexer<List>{});
}

template <typename List>
constexpr auto is_unique_type(const List &) {
    return bool_v<fast::is_unique<List>::value>;
}

template <typename List, typename P>
constexpr auto filter_type(const List &tuple, [[maybe_unused]] P) {
    return tuple.extract(fast::filter_indexer<List, fast::function_to_metafunction<P>::template apply>{});
}

namespace details {
template <int I, typename F, typename... Tuples>
constexpr decltype(auto) zip_with_impl_index(F &&f, Tuples &&... tuples) {
    using ret = decltype(FWD(f)(FWD(tuples).template get<I>()...));
    if constexpr (std::is_void_v<ret>) {
        FWD(f)(FWD(tuples).template get<I>()...);
        return empty_t{};
    } else {
        return FWD(f)(FWD(tuples).template get<I>()...);
    }
}

template <int... Is, typename F, typename... Tuples>
constexpr auto zip_with_impl_all_indices(std::integer_sequence<int, Is...>, F &&f, Tuples &&... tuples) {
    return ltl::tuple_t<decltype(zip_with_impl_index<Is>(FWD(f), FWD(tuples)...))...>{
        zip_with_impl_index<Is>(FWD(f), FWD(tuples)...)...};
}

template <int I, typename... Tuples>
constexpr auto zip_impl_index(Tuples &&... tuples) {
    return ltl::tuple_t<decltype(std::declval<ltl::remove_cvref_t<Tuples>>().template get<I>())...>{
        FWD(tuples).template get<I>()...};
}

template <int... Is, typename... Tuples>
constexpr auto zip_impl_all_indices(std::integer_sequence<int, Is...>, Tuples &&... xs) {
    return ltl::tuple_t{zip_impl_index<Is>(FWD(xs)...)...};
}

} // namespace details

template <typename F, typename T, typename... Tuples>
constexpr auto zip_with(F &&f, T &&tuple, Tuples &&... tuples) {
    static_assert((IsTuple<Tuples> && ... && IsTuple<T>), "All tuples must be tuples");
    constexpr auto length = tuple_size<T>::value;
    static_assert((... && (tuple_size<Tuples>::value == length)), "All tuples must be of the same length");
    return details::zip_with_impl_all_indices(tuple.make_indexer_sequence(), FWD(f), FWD(tuple), FWD(tuples)...);
}

template <typename T, typename... Tuples>
constexpr auto zip_type(T &&tuple, Tuples &&... tuples) {
    static_assert((IsTuple<Tuples> && ... && IsTuple<T>), "All tuples must be tuples");
    constexpr auto length = tuple_size<T>::value;
    static_assert((... && (tuple_size<Tuples>::value == length)), "All tuples must be of the same length");
    return details::zip_impl_all_indices(tuple.make_indexer_sequence(), FWD(tuple), FWD(tuples)...);
}

template <typename F, typename T>
constexpr auto enumerate_with(F &&f, T &&tuple) {
    static_assert(IsTuple<T>, "All tuples must be tuples");
    return zip_with(FWD(f), tuple.make_indexer(), FWD(tuple));
}

template <typename T>
constexpr auto enumerate_type(T &&tuple) {
    static_assert(IsTuple<T>, "All tuples must be tuples");
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
using build_from_type_list = typename build_from_type_listImpl<T, ltl::remove_cvref_t<Ts>>::type;

} // namespace ltl
