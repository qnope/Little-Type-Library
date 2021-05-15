#pragma once

#include "concept.h"
#include "crtp.h"
#include "ltl.h"

#include <tuple>
#include <array>

namespace ltl {

namespace detail {
template <typename T>
using safe_add_lvalue_reference = std::conditional_t<std::is_reference_v<T>, T, std::add_lvalue_reference_t<T>>;

template <int I, typename T>
struct tuple_leaf {
    T value{};
};

template <int I, typename T>
constexpr T &get_leaf(tuple_leaf<I, T> &x) {
    return x.value;
}

template <int I, typename T>
constexpr const T &get_leaf(const tuple_leaf<I, T> &x) {
    return x.value;
}

template <int I, typename T>
constexpr T get_leaf(tuple_leaf<I, T> &&x) {
    return std::move(x).value;
}

template <int I, typename T>
constexpr const T get_leaf(const tuple_leaf<I, T> &&x) {
    return std::move(x).value;
}

template <typename...>
struct tuple_base_t;

template <int... Is, typename... Ts>
struct tuple_base_t<std::integer_sequence<int, Is...>, Ts...> : tuple_leaf<Is, Ts>... {
    constexpr static auto length = number_v<sizeof...(Ts)>;
    constexpr static auto isEmpty = length == 0_n;

    template <typename... _Ts>
    constexpr tuple_base_t &operator=(const tuple_base_t<_Ts...> &t) {
        typed_static_assert_msg(t.length == length, "Tuple must have the same size");
        ((get_leaf<Is>(*this) = get_leaf<Is>(t)), ...);
        return *this;
    }

    template <typename... _Ts>
    constexpr tuple_base_t &operator=(tuple_base_t<_Ts...> &&t) {
        typed_static_assert_msg(t.length == length, "Tuple must have the same size");
        ((get_leaf<Is>(*this) = get_leaf<Is>(std::move(t))), ...);
        return *this;
    }

    template <typename F>
    constexpr decltype(auto) operator()(F &&f) & {
        return ltl::fast_invoke(FWD(f), get_leaf<Is>(*this)...);
    }

    template <typename F>
    constexpr decltype(auto) operator()(F &&f) const & {
        return ltl::fast_invoke(FWD(f), get_leaf<Is>(*this)...);
    }

    template <typename F>
    constexpr decltype(auto) operator()(F &&f) && {
        return ltl::fast_invoke(FWD(f), get_leaf<Is>(std::move(*this))...);
    }

    template <typename... _Ts>
    constexpr auto operator==(const tuple_base_t<_Ts...> &t) const noexcept {
        typed_static_assert_msg(t.length == length, "Tuple must have the same size");
        return ((get_leaf<Is>(*this) == get_leaf<Is>(t)) && ... && true_v);
    }

    template <typename... _Ts>
    constexpr bool operator<(const tuple_base_t<_Ts...> &t) const noexcept {
        typed_static_assert_msg(t.length == length, "Tuple must have the same size");
        bool resultComparison = false;
        auto tester = [&resultComparison](const auto &a, const auto &b) {
            if (a == b) {
                resultComparison = false;
                return true;
            }
            resultComparison = a < b;
            return false;
        };
        (... && (tester(get_leaf<Is>(*this), get_leaf<Is>(t))));
        return resultComparison;
    }
};

template <std::size_t N1, typename SequenceGenerator>
struct make_integer_sequence_impl;

template <std::size_t N1, int... Is>
struct make_integer_sequence_impl<N1, std::integer_sequence<int, Is...>> {
    using type = std::integer_sequence<int, (N1 + Is)...>;
};

template <std::size_t N1, std::size_t N2>
using make_integer_sequence = typename make_integer_sequence_impl<N1, std::make_integer_sequence<int, N2 - N1>>::type;

} // namespace detail

template <typename... Ts>
struct tuple_t;

namespace detail {
template <typename Seq>
struct integer_sequence_to_number_listImpl;

template <int... Is>
struct integer_sequence_to_number_listImpl<std::integer_sequence<int, Is...>> {
    using type = ltl::tuple_t<number_t<Is>...>;
};

} // namespace detail

template <typename Seq>
using integer_sequence_to_number_list = typename detail::integer_sequence_to_number_listImpl<Seq>::type;

template <typename... Ts>
struct [[nodiscard]] tuple_t {
    using indexer_sequence_t = std::make_integer_sequence<int, sizeof...(Ts)>;
    using super = detail::tuple_base_t<indexer_sequence_t, Ts...>;
    super impl;

    static constexpr auto length = super::length;
    static constexpr auto isEmpty = super::isEmpty;

    static constexpr auto getTypes() noexcept { return tuple_t<type_t<Ts>...>{}; }

    template <typename... _Ts>
    tuple_t &operator=(const tuple_t<_Ts...> &t) {
        impl = t.impl;
        return *this;
    }

    template <typename... _Ts>
    tuple_t &operator=(tuple_t<_Ts...> &&t) {
        impl = std::move(t).impl;
        return *this;
    }

    template <int I>
    constexpr decltype(auto) operator[](number_t<I>) & {
        return detail::get_leaf<I>(impl);
    }

    template <int I>
    constexpr decltype(auto) operator[](number_t<I>) const & {
        return detail::get_leaf<I>(impl);
    }

    template <int I>
    constexpr decltype(auto) operator[](number_t<I>) && {
        return detail::get_leaf<I>(std::move(impl));
    }

    template <typename F>
    constexpr decltype(auto) operator()(F &&f) & {
        return impl(FWD(f));
    }

    template <typename F>
    constexpr decltype(auto) operator()(F &&f) const & {
        return impl(FWD(f));
    }

    template <typename F>
    constexpr decltype(auto) operator()(F &&f) && {
        return std::move(impl)(FWD(f));
    }

    template <int I>
        [[nodiscard]] constexpr decltype(auto) get(number_t<I> n) & noexcept {
        typed_static_assert(n < length);
        return detail::get_leaf<I>(impl);
    }

    template <int I>
    [[nodiscard]] constexpr decltype(auto) get(number_t<I> n) const &noexcept {
        typed_static_assert(n < length);
        return detail::get_leaf<I>(impl);
    }

    template <int I>
        [[nodiscard]] constexpr decltype(auto) get(number_t<I> n) && noexcept {
        typed_static_assert(n < length);
        return detail::get_leaf<I>(std::move(impl));
    }

    template <int I>
        [[nodiscard]] constexpr decltype(auto) get() & noexcept {
        return get(number_v<I>);
    }

    template <int N>
    [[nodiscard]] constexpr decltype(auto) get() const &noexcept {
        return get(number_v<N>);
    }

    template <int N>
        [[nodiscard]] constexpr decltype(auto) get() && noexcept {
        return std::move(*this).get(number_v<N>);
    }

    template <int... Is>
    [[nodiscard]] constexpr auto extract(number_t<Is>... ns) const &noexcept {
        return tuple_t<decltype(std::move(*const_cast<tuple_t *>(this))[ns])...>{(*this)[ns]...};
    }

    template <int... Is>
        [[nodiscard]] constexpr auto extract(number_t<Is>... ns) && noexcept {
        return tuple_t<decltype(std::move(*const_cast<tuple_t *>(this))[ns])...>{std::move(*this)[ns]...};
    }

    template <int... Is>
    [[nodiscard]] constexpr auto extract(std::integer_sequence<int, Is...>) const &noexcept {
        return this->extract(number_v<Is>...);
    }

    template <int... Is>
        [[nodiscard]] constexpr auto extract(std::integer_sequence<int, Is...>) && noexcept {
        return std::move(*this).extract(number_v<Is>...);
    }

    template <typename T>
    [[nodiscard]] constexpr auto push_back(T && newValue) const & {
        auto fwdAll = [&newValue](auto &... xs) {
            return tuple_t<Ts..., decay_reference_wrapper_t<T>>{xs..., static_cast<T &&>(newValue)};
        };

        return (*this)(fwdAll);
    }

    template <typename T>
    [[nodiscard]] constexpr auto push_back(T && newValue) && {
        auto fwdAll = [&newValue](Ts &&... xs) {
            return tuple_t<Ts..., decay_reference_wrapper_t<T>>{FWD(xs)..., static_cast<T &&>(newValue)};
        };
        return std::move(*this)(fwdAll);
    }

    template <typename T>
    [[nodiscard]] constexpr auto push_front(T && newValue) const & {
        auto fwdAll = [&newValue](auto &... xs) {
            return tuple_t<decay_reference_wrapper_t<T>, Ts...>{static_cast<T &&>(newValue), xs...};
        };
        return (*this)(fwdAll);
    }

    template <typename T>
    [[nodiscard]] constexpr auto push_front(T && newValue) && {
        auto fwdAll = [&newValue](Ts &&... xs) {
            return tuple_t<decay_reference_wrapper_t<T>, Ts...>{static_cast<T &&>(newValue), FWD(xs)...};
        };
        return std::move(*this)(fwdAll);
    }

    [[nodiscard]] constexpr auto pop_back() const & {
        return this->extract(std::make_integer_sequence<int, length.value - 1>{});
    }

    [[nodiscard]] constexpr auto pop_back() && {
        return std::move(*this).extract(std::make_integer_sequence<int, length.value - 1>{});
    }

    [[nodiscard]] constexpr auto pop_front() const & {
        return this->extract(detail::make_integer_sequence<1, length.value>{});
    }

    [[nodiscard]] constexpr auto pop_front() && {
        return std::move(*this).extract(detail::make_integer_sequence<1, length.value>{});
    }

    static constexpr auto make_indexer_sequence() noexcept { return indexer_sequence_t{}; }

    static constexpr auto make_indexer() noexcept { return integer_sequence_to_number_list<indexer_sequence_t>{}; }

    template <typename... _Ts>
    constexpr auto operator==(const tuple_t<_Ts...> &x) const {
        return impl == x.impl;
    }

    template <typename... _Ts>
    constexpr auto operator<(const tuple_t<_Ts...> &x) const {
        return impl < x.impl;
    }

    LTL_CRTP_COMPARABLE(tuple_t)
};

template <typename... Ts>
tuple_t(Ts...)->tuple_t<decay_reference_wrapper_t<Ts>...>;

////////////////////// Templates
/// Convenience types
template <typename... Types>
using type_list_t = tuple_t<type_t<Types>...>;
template <int... Ns>
using number_list_t = tuple_t<number_t<Ns>...>;
template <bool... Bs>
using bool_list_t = tuple_t<bool_t<Bs>...>;

// Convenience variables
template <typename... Types>
constexpr type_list_t<Types...> type_list_v{};
template <int... Ns>
constexpr number_list_t<Ns...> number_list_v{};
template <bool... Bs>
constexpr bool_list_t<Bs...> bool_list_v{};

template <typename... Ts>
tuple_t<Ts &...> tie(Ts &... ts) noexcept {
    return {ts...};
}

LTL_MAKE_IS_KIND(tuple_t, is_tuple, is_tuple_t, IsTuple, typename, ...);
LTL_MAKE_IS_KIND(type_list_t, is_type_list, is_type_list_t, IsTypeList, typename, ...);
LTL_MAKE_IS_KIND(number_list_t, is_number_list, is_number_list_t, IsNumberList, int, ...);
LTL_MAKE_IS_KIND(bool_list_t, is_bool_list, is_bool_list_t, IsBoolList, bool, ...);

template <typename N1, typename N2>
[[nodiscard]] constexpr auto build_index_list(N1, N2) {
    return integer_sequence_to_number_list<detail::make_integer_sequence<N1::value, N2::value>>{};
}

template <typename N>
[[nodiscard]] constexpr auto build_index_list(N n) {
    return build_index_list(number_v<0>, n);
}

template <const auto &array, typename = std::make_index_sequence<std::tuple_size_v<std::decay_t<decltype(array)>>>>
struct array_to_index_sequence;

template <const auto &array, std::size_t... Is>
struct array_to_index_sequence<array, std::index_sequence<Is...>> {
    using type = std::integer_sequence<std::decay_t<decltype(array[0])>, array[Is]...>;
};

template <const auto &array>
using array_to_index_sequence_t = typename array_to_index_sequence<array>::type;

template <typename... T1, typename... T2>
constexpr auto operator+(const tuple_t<T1...> &t1, const tuple_t<T2...> &t2) {
    constexpr auto indices1 = build_index_list(number_v<sizeof...(T1)>);
    constexpr auto indices2 = build_index_list(number_v<sizeof...(T2)>);

    return indices1([indices2, &t1, &t2](auto... n1s) {
        return indices2([&t1, &t2, n1s...](auto... n2s) { return tuple_t<T1..., T2...>{t1[n1s]..., t2[n2s]...}; });
    });
}

template <typename F, typename Tuple, requires_f(IsTuple<Tuple>)>
constexpr decltype(auto) apply(F &&f, Tuple &&tuple) {
    return FWD(tuple)(FWD(f));
}

template <typename F, typename Tuple, requires_f(IsTuple<Tuple>)>
F for_each(Tuple &&tuple, F &&f) {
    auto retrieveAllArgs = [&f](auto &&... xs) { (static_cast<F &&>(f)(FWD(xs)), ...); };
    FWD(tuple)(retrieveAllArgs);
    return FWD(f);
}

} // namespace ltl

namespace std {
template <typename... Ts>
struct tuple_size<::ltl::tuple_t<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template <std::size_t I, typename... Ts>
struct tuple_element<I, ::ltl::tuple_t<Ts...>> {
    using type = decltype(std::declval<::ltl::tuple_t<Ts...>>().template get<I>());
};

template <std::size_t I, typename... Ts>
struct tuple_element<I, const ::ltl::tuple_t<Ts...>> {
    using type = decltype(std::declval<const ::ltl::tuple_t<Ts...>>().template get<I>());
};

template <std::size_t N, typename Tuple, requires_f(::ltl::IsTuple<Tuple>)>
decltype(auto) get(Tuple &&tuple) {
    return FWD(tuple)[number_v<N>];
}

} // namespace std
