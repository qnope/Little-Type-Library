/**
 * @file Tuple.h
 */
#pragma once

#include "concept.h"
#include "crtp.h"
#include "fast.h"

#include <array>

namespace ltl {
template <typename... Ts>
struct tuple_t;

namespace detail {
template <int... Is, typename F, typename T>
constexpr decltype(auto) apply_impl(std::integer_sequence<int, Is...>, F &&f, T &&t) {
    return FWD(f)(FWD(t)[number_v<Is>]...);
}

} // namespace detail

template <int... Is, typename F>
constexpr decltype(auto) execute_with_indices(std::integer_sequence<int, Is...>, F &&f) {
    return FWD(f)(number_v<Is>...);
}

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
struct tuple_base_t<std::integer_sequence<int, Is...>, Ts...> : tuple_leaf<Is, Ts>... {};

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
/**
 * @brief The tuple_t struct
 */
struct [[nodiscard]] tuple_t {
    using indexer_sequence_t = std::make_integer_sequence<int, sizeof...(Ts)>;
    using super = detail::tuple_base_t<indexer_sequence_t, Ts...>;
    super impl{};

    constexpr static auto length = number_v<sizeof...(Ts)>;
    constexpr static auto isEmpty = length == 0_n;

    static constexpr auto getTypes() noexcept { return fast::type_list<Ts...>{}; }

    template <typename... _Ts>
    tuple_t &operator=(const tuple_t<_Ts...> &t) {
        execute_with_indices(indexer_sequence_t{}, [&](auto... indices) { //
            (((*this)[indices] = t[indices]), ...);
        });
        return *this;
    }

    template <typename... _Ts>
    tuple_t &operator=(tuple_t<_Ts...> &&t) {
        execute_with_indices(indexer_sequence_t{}, [&](auto... indices) { //
            (((*this)[indices] = std::move(t)[indices]), ...);
        });
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
        return ::ltl::detail::apply_impl(indexer_sequence_t{}, FWD(f), *this);
    }

    template <typename F>
    constexpr decltype(auto) operator()(F &&f) const & {
        return ::ltl::detail::apply_impl(indexer_sequence_t{}, FWD(f), *this);
    }

    template <typename F>
    constexpr decltype(auto) operator()(F &&f) && {
        return ::ltl::detail::apply_impl(indexer_sequence_t{}, FWD(f), std::move(*this));
    }

    template <int I>
    [[nodiscard]] constexpr decltype(auto) get(number_t<I> n) &noexcept {
        typed_static_assert(n < length);
        return detail::get_leaf<I>(impl);
    }

    template <int I>
    [[nodiscard]] constexpr decltype(auto) get(number_t<I> n) const &noexcept {
        typed_static_assert(n < length);
        return detail::get_leaf<I>(impl);
    }

    template <int I>
    [[nodiscard]] constexpr decltype(auto) get(number_t<I> n) &&noexcept {
        typed_static_assert(n < length);
        return detail::get_leaf<I>(std::move(impl));
    }

    template <int I>
    [[nodiscard]] constexpr decltype(auto) get() &noexcept {
        static_assert(I < length.value);
        return detail::get_leaf<I>(impl);
    }

    template <int I>
    [[nodiscard]] constexpr decltype(auto) get() const &noexcept {
        static_assert(I < length.value);
        return detail::get_leaf<I>(impl);
    }

    template <int I>
    [[nodiscard]] constexpr decltype(auto) get() &&noexcept {
        static_assert(I < length.value);
        return detail::get_leaf<I>(std::move(impl));
    }

    template <int... Is>
    [[nodiscard]] constexpr auto extract(number_t<Is>...) const &noexcept {
        return this->extract(std::integer_sequence<int, Is...>{});
    }

    template <int... Is>
    [[nodiscard]] constexpr auto extract(number_t<Is>...) &&noexcept {
        return std::move(*this).extract(std::integer_sequence<int, Is...>{});
    }

    template <int... Is>
    [[nodiscard]] constexpr auto extract(std::integer_sequence<int, Is...>) const &noexcept {
        return tuple_t<decltype(detail::get_leaf<Is>(std::declval<super>()))...>{{detail::get_leaf<Is>(impl)...}};
    }

    template <int... Is>
    [[nodiscard]] constexpr auto extract(std::integer_sequence<int, Is...>) &&noexcept {
        return tuple_t<decltype(detail::get_leaf<Is>(std::declval<super>()))...>{
            {detail::get_leaf<Is>(std::move(impl))...}};
    }

    template <typename T>
    [[nodiscard]] constexpr auto push_back(T &&newValue) const & {
        auto fwdAll = [&newValue](auto &&...xs) {
            return tuple_t<Ts..., decay_reference_wrapper_t<T>>{{FWD(xs)..., FWD(newValue)}};
        };
        return detail::apply_impl(indexer_sequence_t{}, fwdAll, *this);
    }

    template <typename T>
    [[nodiscard]] constexpr auto push_back(T &&newValue) && {
        auto fwdAll = [&newValue](Ts &&...xs) {
            return tuple_t<Ts..., decay_reference_wrapper_t<T>>{{FWD(xs)..., FWD(newValue)}};
        };
        return detail::apply_impl(indexer_sequence_t{}, fwdAll, std::move(*this));
    }

    template <typename T>
    [[nodiscard]] constexpr auto push_front(T &&newValue) const & {
        auto fwdAll = [&newValue](auto &&...xs) {
            return tuple_t<decay_reference_wrapper_t<T>, Ts...>{{FWD(newValue), FWD(xs)...}};
        };
        return detail::apply_impl(indexer_sequence_t{}, fwdAll, *this);
    }

    template <typename T>
    [[nodiscard]] constexpr auto push_front(T &&newValue) && {
        auto fwdAll = [&newValue](Ts &&...xs) {
            return tuple_t<decay_reference_wrapper_t<T>, Ts...>{{FWD(newValue), FWD(xs)...}};
        };
        return detail::apply_impl(indexer_sequence_t{}, fwdAll, std::move(*this));
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
    constexpr auto operator==(const tuple_t<_Ts...> &t) const {
        typed_static_assert_msg(t.length == length, "Tuple must have the same size");
        return execute_with_indices(indexer_sequence_t{}, [&](auto... indices) { //
            return (((*this)[indices] == t[indices]) && ... && true_v);
        });
    }

    template <typename... _Ts>
    constexpr auto operator<(const tuple_t<_Ts...> &t) const {
        typed_static_assert_msg(t.length == length, "Tuple must have the same size");
        return execute_with_indices(indexer_sequence_t{}, [&](auto... indices) {
            bool resultComparison = false;
            auto tester = [&resultComparison](const auto &a, const auto &b) {
                if (a == b) {
                    resultComparison = false;
                    return true;
                }
                resultComparison = a < b;
                return false;
            };
            (... && (tester((*this)[indices], t[indices])));
            return resultComparison;
        });
    }

    LTL_CRTP_COMPARABLE(tuple_t)
};

template <typename... Ts>
tuple_t(Ts...) -> tuple_t<decay_reference_wrapper_t<Ts>...>;

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
tuple_t<Ts &...> tie(Ts &...ts) noexcept {
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

template <typename... T1, typename... T2>
constexpr auto operator+(const tuple_t<T1...> &t1, const tuple_t<T2...> &t2) {
    return ::ltl::execute_with_indices(t1.make_indexer_sequence(), [&](auto... n1s) {
        return ::ltl::execute_with_indices(t2.make_indexer_sequence(), [&](auto... n2s) {
            return tuple_t<T1..., T2...>{t1[n1s]..., t2[n2s]...};
        });
    });
}

template <typename F, typename Tuple, requires_f(IsTuple<Tuple>)>
constexpr decltype(auto) apply(F &&f, Tuple &&tuple) {
    constexpr auto indexer = ltl::remove_cvref_t<Tuple>::make_indexer_sequence();
    return detail::apply_impl(indexer, FWD(f), FWD(tuple));
}

template <typename F, typename Tuple, requires_f(IsTuple<Tuple>)>
F for_each(Tuple &&tuple, F &&f) {
    auto retrieveAllArgs = [&f](auto &&...xs) { (static_cast<F &&>(f)(FWD(xs)), ...); };
    FWD(tuple)(retrieveAllArgs);
    return FWD(f);
}

template <typename Tuple>
class tuple_size {
    template <typename T>
    struct impl;

    template <typename... Ts>
    struct impl<tuple_t<Ts...>> {
        static constexpr auto value = sizeof...(Ts);
    };

  public:
    static constexpr auto value = impl<ltl::remove_cvref_t<Tuple>>::value;
};

template <typename Tuple>
inline constexpr auto tuple_size_v = tuple_size<Tuple>::value;

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
