#pragma once

#include <utility>

#include "concept.h"
#include "crtp.h"
#include "ltl.h"

namespace ltl {
namespace detail {
template <typename T>
using safe_add_lvalue_reference = std::conditional_t<std::is_reference_v<T>, T, std::add_lvalue_reference_t<T>>;

template <typename T>
using safe_add_rvalue_reference = std::conditional_t<std::is_reference_v<T>, T, std::add_rvalue_reference_t<T>>;

template <int I, typename T>
struct Value {
    constexpr Value() noexcept : m_value{} {}
    constexpr Value(T &&t) noexcept : m_value{FWD(t)} {}
    constexpr Value(const Value &v) noexcept : m_value{v.m_value} {}
    constexpr Value(Value &&v) noexcept : m_value{FWD(v.m_value)} {}

    template <typename _T, typename = std::enable_if_t<ltl::type_v<std::decay_t<_T>> != ltl::type_v<Value>>>
    constexpr Value(_T &&t) noexcept : m_value{FWD(t)} {}

    constexpr Value &operator=(Value v) noexcept {
        m_value = std::move(v.m_value);
        return *this;
    }

    template <typename _T, typename = std::enable_if_t<ltl::type_v<std::decay_t<_T>> != ltl::type_v<Value>>>
    constexpr Value &operator=(_T &&t) noexcept {
        m_value = FWD(t);
        return *this;
    }

    constexpr T operator[](ltl::number_t<I>) && noexcept { return static_cast<safe_add_rvalue_reference<T>>(m_value); }

    constexpr safe_add_lvalue_reference<std::add_const_t<T>> operator[](ltl::number_t<I>) const &noexcept {
        return static_cast<safe_add_lvalue_reference<std::add_const_t<T>>>(m_value);
    }

    constexpr safe_add_lvalue_reference<T> operator[](ltl::number_t<I>) & noexcept {
        return static_cast<safe_add_lvalue_reference<T>>(m_value);
    }

    T m_value;
};

template <typename...>
class tuple_t;

template <int... Is, typename... Ts>
class tuple_t<std::integer_sequence<int, Is...>, Ts...> :
    public Value<Is, Ts>...,
    public Comparable<tuple_t<std::integer_sequence<int, Is...>, Ts...>> {
  public:
    using Value<Is, Ts>::operator[]...;

    constexpr static auto length = number_v<sizeof...(Ts)>;
    constexpr static auto isEmpty = length == 0_n;

    template <bool isNotEmpty = !isEmpty, typename = std::enable_if_t<isNotEmpty>>
    constexpr tuple_t() noexcept {}

    constexpr tuple_t(Ts... ts) : Value<Is, Ts>{FWD(ts)}... {}

    template <typename... _Ts>
    tuple_t &operator=(const tuple_t<_Ts...> &t) {
        typed_static_assert_msg(t.length == length, "Tuple must have the same size");
        (((*this)[number_v<Is>] = t[number_v<Is>]), ...);
        return *this;
    }

    template <typename... _Ts>
    tuple_t &operator=(tuple_t<_Ts...> &&t) {
        typed_static_assert_msg(t.length == length, "Tuple must have the same size");
        (((*this)[number_v<Is>] = std::move(t)[number_v<Is>]), ...);
        return *this;
    }

    template <typename F>
    constexpr decltype(auto) operator()(F &&f) & {
        return ltl::invoke(FWD(f), (*this)[number_v<Is>]...);
    }

    template <typename F>
    constexpr decltype(auto) operator()(F &&f) const & {
        return ltl::invoke(FWD(f), (*this)[number_v<Is>]...);
    }

    template <typename F>
    constexpr decltype(auto) operator()(F &&f) && {
        return ltl::invoke(FWD(f), std::move(*this)[number_v<Is>]...);
    }

    template <int N>
        [[nodiscard]] constexpr decltype(auto) get(number_t<N> n) & noexcept {
        typed_static_assert(n < length);
        return (*this)[n];
    }

    template <int N>
    [[nodiscard]] constexpr decltype(auto) get(number_t<N> n) const &noexcept {
        typed_static_assert(n < length);
        return (*this)[n];
    }

    template <int N>
        [[nodiscard]] constexpr decltype(auto) get(number_t<N> n) && noexcept {
        typed_static_assert(n < length);
        return std::move(*this)[n];
    }

    template <int N>
        [[nodiscard]] constexpr decltype(auto) get() & noexcept {
        return get(number_v<N>);
    }

    template <int N>
    [[nodiscard]] constexpr decltype(auto) get() const &noexcept {
        return get(number_v<N>);
    }

    template <int N>
        [[nodiscard]] constexpr decltype(auto) get() && noexcept {
        return std::move(*this).get(number_v<N>);
    }

    template <typename... _Ts>
    constexpr auto operator==(const tuple_t<_Ts...> &t) const noexcept {
        typed_static_assert_msg(t.length == length, "Tuple must have the same size");
        return (((*this)[number_v<Is>] == t[number_v<Is>]) && ... && true_v);
    }

    template <typename... _Ts>
    constexpr bool operator<(const tuple_t<_Ts...> &t) const noexcept {
        typed_static_assert_msg(t.length == length, "Tuple must have the same size");
        bool resultComparison = false;
        auto tester = [&resultComparison](const auto &a, const auto &b) {
            if (a > b) {
                resultComparison = false;
                return false;
            } else if (a < b) {
                resultComparison = true;
                return false;
            }
            return true;
        };
        (... && (tester((*this)[number_v<Is>], t[number_v<Is>])));
        return resultComparison;
    }
};

} // namespace detail

template <typename... Ts>
class tuple_t : public detail::tuple_t<std::make_integer_sequence<int, sizeof...(Ts)>, Ts...> {
  public:
    using super = detail::tuple_t<std::make_integer_sequence<int, sizeof...(Ts)>, Ts...>;

    using super::isEmpty;
    using super::length;
    using super::super;
    using super::operator=;

    static constexpr auto getTypes() noexcept { return tuple_t<type_t<Ts>...>{}; }

    template <int... Is>
    [[nodiscard]] constexpr auto extract(number_t<Is>... ns) const &noexcept {
        return tuple_t<decltype(std::move(*const_cast<tuple_t *>(this))[ns])...>{(*this)[ns]...};
    }

    template <int... Is>
        [[nodiscard]] constexpr auto extract(number_t<Is>... ns) && noexcept {
        return tuple_t<decltype(std::move(*const_cast<tuple_t *>(this))[ns])...>{std::move(*this)[ns]...};
    }

    template <typename T>
    [[nodiscard]] constexpr auto push_back(T &&newValue) const & {
        auto fwdAll = [&newValue](auto &... xs) {
            return tuple_t<Ts..., decay_reference_wrapper_t<T>>{xs..., std::forward<T>(newValue)};
        };

        return (*this)(fwdAll);
    }

    template <typename T>
    [[nodiscard]] constexpr auto push_back(T &&newValue) && {
        auto fwdAll = [&newValue](Ts &&... xs) {
            return tuple_t<Ts..., decay_reference_wrapper_t<T>>{FWD(xs)..., std::forward<T>(newValue)};
        };
        return std::move(*this)(fwdAll);
    }

    template <typename T>
    [[nodiscard]] constexpr auto push_front(T &&newValue) const & {
        auto fwdAll = [&newValue](auto &... xs) {
            return tuple_t<decay_reference_wrapper_t<T>, Ts...>{std::forward<T>(newValue), xs...};
        };
        return (*this)(fwdAll);
    }

    template <typename T>
    [[nodiscard]] constexpr auto push_front(T &&newValue) && {
        auto fwdAll = [&newValue](Ts &&... xs) {
            return tuple_t<decay_reference_wrapper_t<T>, Ts...>{std::forward<T>(newValue), FWD(xs)...};
        };
        return std::move(*this)(fwdAll);
    }

    [[nodiscard]] constexpr auto pop_back() const & {
        auto extracter = [this](auto... numbers) { return this->extract(numbers...); };
        constexpr auto numbers = build_index_sequence(length - 1_n);
        return numbers(extracter);
    }

    [[nodiscard]] constexpr auto pop_back() && {
        auto extracter = [this](auto... numbers) { return std::move(*this).extract(numbers...); };
        constexpr auto numbers = build_index_sequence(length - 1_n);
        return numbers(extracter);
    }

    [[nodiscard]] constexpr auto pop_front() const & {
        auto extracter = [this](auto... numbers) { return this->extract(numbers...); };
        constexpr auto numbers = build_index_sequence(1_n, length);
        return apply(numbers, extracter);
    }

    [[nodiscard]] constexpr auto pop_front() && {
        auto extracter = [this](auto... numbers) { return std::move(*this).extract(numbers...); };
        constexpr auto numbers = build_index_sequence(1_n, length);
        return apply(numbers, extracter);
    }

    static constexpr auto make_indexer() noexcept { return build_index_sequence(length); }

    template <typename N1, typename N2>
    [[nodiscard]] static constexpr auto build_index_sequence(N1 n1, N2 n2) {
        return build_index_sequence_helper(n1, n2);
    }

    template <typename N>
    [[nodiscard]] static constexpr auto build_index_sequence(N n) {
        return build_index_sequence(0_n, n);
    }

  private:
    template <int N, int... Ns>
    [[nodiscard]] static constexpr auto build_index_sequence_helper(number_t<N>, std::integer_sequence<int, Ns...>) {
        return tuple_t<number_t<N + Ns>...>{};
    }

    template <int N1, int N2>
    [[nodiscard]] static constexpr auto build_index_sequence_helper(number_t<N1> n1, number_t<N2> n2) {
        typed_static_assert_msg(n1 <= n2, "n1 must be lesser or equal to n2");
        return build_index_sequence_helper(n1, std::make_integer_sequence<int, N2 - N1>{});
    }
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

LTL_MAKE_IS_KIND(tuple_t, is_tuple_t, IsTuple, typename);
LTL_MAKE_IS_KIND(type_list_t, is_type_list_t, IsTypeList, typename);
LTL_MAKE_IS_KIND(number_list_t, is_number_list_t, IsNumberList, int);
LTL_MAKE_IS_KIND(bool_list_t, is_bool_list_t, IsBoolList, bool);

template <typename N1, typename N2>
[[nodiscard]] constexpr auto build_index_sequence(N1 n1, N2 n2) {
    return tuple_t<>::build_index_sequence(n1, n2);
}

template <typename N>
[[nodiscard]] constexpr auto build_index_sequence(N n) {
    return tuple_t<>::build_index_sequence(0_n, n);
}

template <typename... T1, typename... T2>
constexpr auto operator+(const tuple_t<T1...> &t1, const tuple_t<T2...> &t2) {
    constexpr auto indices1 = build_index_sequence(number_v<sizeof...(T1)>);
    constexpr auto indices2 = build_index_sequence(number_v<sizeof...(T2)>);

    return indices1([indices2, &t1, &t2](auto... n1s) {
        return indices2([&t1, &t2, n1s...](auto... n2s) { return tuple_t<T1..., T2...>{t1[n1s]..., t2[n2s]...}; });
    });
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

#define FROM_VARIADIC(args)                                                                                            \
    ::ltl::tuple_t<decltype(args)...> { args... }

#define TO_VARIADIC(tuple, var, expr) tuple([](auto &&... var) { expr; });
#define TO_VARIADIC_RETURN(tuple, var, expr) tuple([](auto &&... var) { return expr; });
