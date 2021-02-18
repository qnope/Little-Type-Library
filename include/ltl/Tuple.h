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

template <int I, typename T, bool = std::is_empty_v<T> && !std::is_final_v<T>>
struct value {
    constexpr explicit value() noexcept : m_value{} {}

    template <typename _T>
    constexpr explicit value(_T &&t) noexcept : m_value{FWD(t)} {}

    constexpr value(const value &value) :
        m_value{static_cast<std::conditional_t<std::is_reference_v<T>, T, const T &>>(value.m_value)} {}

    constexpr value(value &&) = default;
    constexpr value &operator=(const value &) = default;
    constexpr value &operator=(value &&) = default;

    constexpr T operator[](ltl::number_t<I>) &&noexcept { return FWD(m_value); }

    constexpr safe_add_lvalue_reference<std::add_const_t<T>> operator[](ltl::number_t<I>) const &noexcept {
        return static_cast<safe_add_lvalue_reference<std::add_const_t<T>>>(m_value);
    }

    constexpr safe_add_lvalue_reference<T> operator[](ltl::number_t<I>) &noexcept {
        return static_cast<safe_add_lvalue_reference<T>>(m_value);
    }

    T m_value;
};

template <int I, typename T>
struct value<I, T, true> : private T {
    constexpr explicit value() noexcept {}

    template <typename _T>
    constexpr explicit value(_T &&t) noexcept : T{FWD(t)} {}

    constexpr T &operator[](ltl::number_t<I>) &noexcept { return *this; }
    constexpr const T &operator[](ltl::number_t<I>) const &noexcept { return *this; }

    constexpr T operator[](ltl::number_t<I>) &&noexcept { return static_cast<T &&>(*this); }
};

template <typename...>
class tuple_base_t;

template <int... Is, typename... Ts>
class tuple_base_t<std::integer_sequence<int, Is...>, Ts...> : public value<Is, Ts>... {
  public:
    using value<Is, Ts>::operator[]...;

    constexpr static auto length = number_v<sizeof...(Ts)>;
    constexpr static auto isEmpty = length == 0_n;

    template <bool isNotEmpty = !isEmpty, typename = std::enable_if_t<isNotEmpty>>
    constexpr tuple_base_t() noexcept {}

    constexpr tuple_base_t(Ts... ts) : value<Is, Ts>{FWD(ts)}... {}

    template <typename... _Ts>
    tuple_base_t &operator=(const tuple_base_t<_Ts...> &t) {
        typed_static_assert_msg(t.length == length, "Tuple must have the same size");
        (((*this)[number_v<Is>] = t[number_v<Is>]), ...);
        return *this;
    }

    template <typename... _Ts>
    tuple_base_t &operator=(tuple_base_t<_Ts...> &&t) {
        typed_static_assert_msg(t.length == length, "Tuple must have the same size");
        (((*this)[number_v<Is>] = std::move(t)[number_v<Is>]), ...);
        return *this;
    }

    template <typename F>
    constexpr decltype(auto) operator()(F &&f) & {
        return ltl::fast_invoke(FWD(f), (*this)[number_v<Is>]...);
    }

    template <typename F>
    constexpr decltype(auto) operator()(F &&f) const & {
        return ltl::fast_invoke(FWD(f), (*this)[number_v<Is>]...);
    }

    template <typename F>
    constexpr decltype(auto) operator()(F &&f) && {
        return ltl::fast_invoke(FWD(f), std::move(*this)[number_v<Is>]...);
    }

    template <int N>
    [[nodiscard]] constexpr decltype(auto) get(number_t<N> n) &noexcept {
        typed_static_assert(n < length);
        return (*this)[n];
    }

    template <int N>
    [[nodiscard]] constexpr decltype(auto) get(number_t<N> n) const &noexcept {
        typed_static_assert(n < length);
        return (*this)[n];
    }

    template <int N>
    [[nodiscard]] constexpr decltype(auto) get(number_t<N> n) &&noexcept {
        typed_static_assert(n < length);
        return std::move(*this)[n];
    }

    template <int N>
    [[nodiscard]] constexpr decltype(auto) get() &noexcept {
        return get(number_v<N>);
    }

    template <int N>
    [[nodiscard]] constexpr decltype(auto) get() const &noexcept {
        return get(number_v<N>);
    }

    template <int N>
    [[nodiscard]] constexpr decltype(auto) get() &&noexcept {
        return std::move(*this).get(number_v<N>);
    }

    template <typename... _Ts>
    constexpr auto operator==(const tuple_base_t<_Ts...> &t) const noexcept {
        typed_static_assert_msg(t.length == length, "Tuple must have the same size");
        return (((*this)[number_v<Is>] == t[number_v<Is>]) && ... && true_v);
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
        (... && (tester((*this)[number_v<Is>], t[number_v<Is>])));
        return resultComparison;
    }
};

template <typename Sequence, typename... Ts>
struct simple_tuple_base_t;

template <int I, typename T>
struct simple_value {
    constexpr ltl::type_t<T> operator[](ltl::number_t<I>) const noexcept { return {}; }
};

template <int... Is, typename... Ts>
struct simple_tuple_base_t<std::integer_sequence<int, Is...>, Ts...> : simple_value<Is, Ts>... {
    constexpr static auto length = number_v<sizeof...(Ts)>;
    constexpr static auto isEmpty = length == 0_n;

    using simple_value<Is, Ts>::operator[]...;

    constexpr simple_tuple_base_t() noexcept {}

    template <typename F>
    constexpr auto operator()(F &&f) const noexcept {
        return ltl::fast_invoke(FWD(f), type_v<Ts>...);
    }

    template <int N>
    [[nodiscard]] constexpr auto get(number_t<N> n) const noexcept {
        typed_static_assert(n < length);
        return (*this)[n];
    }

    template <int N>
    [[nodiscard]] constexpr auto get() const noexcept {
        return get(number_v<N>);
    }

    template <typename Sequence, typename... _Ts>
    constexpr auto operator==(simple_tuple_base_t<Sequence, _Ts...>) const noexcept {
        return ltl::bool_v<(true && ... && (std::is_same_v<Ts, _Ts>))>;
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
class tuple_t;

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
class [[nodiscard]] tuple_t :
    public detail::tuple_base_t<std::make_integer_sequence<int, sizeof...(Ts)>, Ts...>,
    public crtp::Comparable<tuple_t<Ts...>> {
  public:
    using indexer_sequence_t = std::make_integer_sequence<int, sizeof...(Ts)>;
    using super = detail::tuple_base_t<indexer_sequence_t, Ts...>;

    using super::isEmpty;
    using super::length;
    using super::super;

    template <typename... _Ts>
    tuple_t &operator=(const tuple_t<_Ts...> &t) {
        static_cast<super &>(*this) = t;
        return *this;
    }

    template <typename... _Ts>
    tuple_t &operator=(tuple_t<_Ts...> &&t) {
        static_cast<super &>(*this) = std::move(t);
        return *this;
    }

    static constexpr auto getTypes() noexcept { return tuple_t<type_t<Ts>...>{}; }

    template <int... Is>
    [[nodiscard]] constexpr auto extract(number_t<Is>... ns) const &noexcept {
        return tuple_t<decltype(std::move(*const_cast<tuple_t *>(this))[ns])...>{(*this)[ns]...};
    }

    template <int... Is>
    [[nodiscard]] constexpr auto extract(number_t<Is>... ns) &&noexcept {
        return tuple_t<decltype(std::move(*const_cast<tuple_t *>(this))[ns])...>{std::move(*this)[ns]...};
    }

    template <int... Is>
    [[nodiscard]] constexpr auto extract(std::integer_sequence<int, Is...>) const &noexcept {
        return this->extract(number_v<Is>...);
    }

    template <int... Is>
    [[nodiscard]] constexpr auto extract(std::integer_sequence<int, Is...>) &&noexcept {
        return std::move(*this).extract(number_v<Is>...);
    }

    template <typename T>
    [[nodiscard]] constexpr auto push_back(T &&newValue) const & {
        auto fwdAll = [&newValue](auto &...xs) {
            return tuple_t<Ts..., decay_reference_wrapper_t<T>>{xs..., static_cast<T &&>(newValue)};
        };

        return (*this)(fwdAll);
    }

    template <typename T>
    [[nodiscard]] constexpr auto push_back(T &&newValue) && {
        auto fwdAll = [&newValue](Ts &&...xs) {
            return tuple_t<Ts..., decay_reference_wrapper_t<T>>{FWD(xs)..., static_cast<T &&>(newValue)};
        };
        return std::move(*this)(fwdAll);
    }

    template <typename T>
    [[nodiscard]] constexpr auto push_front(T &&newValue) const & {
        auto fwdAll = [&newValue](auto &...xs) {
            return tuple_t<decay_reference_wrapper_t<T>, Ts...>{static_cast<T &&>(newValue), xs...};
        };
        return (*this)(fwdAll);
    }

    template <typename T>
    [[nodiscard]] constexpr auto push_front(T &&newValue) && {
        auto fwdAll = [&newValue](Ts &&...xs) {
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
};

template <typename... Ts>
class tuple_t<ltl::type_t<Ts>...> :
    public detail::simple_tuple_base_t<std::make_integer_sequence<int, sizeof...(Ts)>, Ts...>,
    public crtp::Comparable<tuple_t<ltl::type_t<Ts>...>> {
  public:
    using indexer_sequence_t = std::make_integer_sequence<int, sizeof...(Ts)>;
    using super = detail::simple_tuple_base_t<indexer_sequence_t, Ts...>;
    using super::isEmpty;
    using super::length;

    template <bool isNotEmpty = !isEmpty, typename = std::enable_if_t<isNotEmpty>>
    constexpr tuple_t() noexcept {}

    constexpr tuple_t(type_t<Ts>...) noexcept {}

    template <int... Is>
    [[nodiscard]] constexpr auto extract(number_t<Is>... ns) const noexcept {
        return tuple_t<decltype((*this)[ns])...>{(*this)[ns]...};
    }

    template <int... Is>
    [[nodiscard]] constexpr auto extract(std::integer_sequence<int, Is...>) const noexcept {
        return this->extract(number_v<Is>...);
    }

    template <typename T>
    [[nodiscard]] constexpr auto push_back(T &&t) const noexcept {
        return tuple_t<type_t<Ts>..., decay_reference_wrapper_t<T>>{type_v<Ts>..., FWD(t)};
    }

    template <typename T>
    [[nodiscard]] constexpr auto push_front(T &&t) const noexcept {
        return tuple_t<decay_reference_wrapper_t<T>, type_t<Ts>...>{FWD(t), type_v<Ts>...};
    }

    [[nodiscard]] constexpr auto pop_back() const noexcept {
        return this->extract(std::make_integer_sequence<int, length.value - 1>{});
    }
    [[nodiscard]] constexpr auto pop_front() const noexcept {
        return this->extract(detail::make_integer_sequence<1, length.value>{});
    }

    static constexpr auto make_indexer_sequence() noexcept { return indexer_sequence_t{}; }

    static constexpr auto make_indexer() noexcept { return integer_sequence_to_number_list<indexer_sequence_t>{}; }
};

template <template <auto> typename V, auto... values>
class tuple_t<V<values>...> {
  public:
    constexpr static auto length = number_v<sizeof...(values)>;
    constexpr static auto isEmpty = length == 0_n;

    template <bool isNotEmpty = !isEmpty, typename = std::enable_if_t<isNotEmpty>>
    constexpr tuple_t() noexcept {}

    constexpr tuple_t(V<values>...) noexcept {}

    static constexpr auto getTypes() noexcept { return tuple_t{}; }

    template <typename F>
    constexpr auto operator()(F &&f) const noexcept {
        return ltl::fast_invoke(FWD(f), V<values>{}...);
    }

    template <int N>
    constexpr auto operator[](ltl::number_t<N>) const noexcept {
        return V<m_array[N]>{};
    }

    template <int N>
    [[nodiscard]] constexpr auto get(number_t<N> n) const noexcept {
        typed_static_assert(n < length);
        return (*this)[n];
    }

    template <int N>
    [[nodiscard]] constexpr auto get() const noexcept {
        return get(number_v<N>);
    }

    template <auto... _values>
    constexpr auto operator==(tuple_t<V<_values>...>) const noexcept {
        return ltl::bool_v<(true && ... && (values == _values))>;
    }

    template <int... Is>
    [[nodiscard]] constexpr auto extract(number_t<Is>... ns) const noexcept {
        return tuple_t<decltype((*this)[ns])...>{(*this)[ns]...};
    }

    template <int... Is>
    [[nodiscard]] constexpr auto extract(std::integer_sequence<int, Is...>) const noexcept {
        return this->extract(number_v<Is>...);
    }

    template <typename T>
    [[nodiscard]] constexpr auto push_back(T &&t) const noexcept {
        return tuple_t<V<values>..., decay_reference_wrapper_t<T>>{V<values>{}..., FWD(t)};
    }

    template <typename T>
    [[nodiscard]] constexpr auto push_front(T &&t) const noexcept {
        return tuple_t<decay_reference_wrapper_t<T>, V<values>...>{FWD(t), V<values>{}...};
    }

    [[nodiscard]] constexpr auto pop_back() const noexcept {
        return this->extract(std::make_integer_sequence<int, length.value - 1>{});
    }
    [[nodiscard]] constexpr auto pop_front() const noexcept {
        return this->extract(detail::make_integer_sequence<1, length.value>{});
    }

    static constexpr auto make_indexer_sequence() noexcept { return std::make_integer_sequence<int, length.value>{}; }

    static constexpr auto make_indexer() noexcept {
        return integer_sequence_to_number_list<std::make_integer_sequence<int, length.value>>{};
    }

  private:
    static constexpr std::array m_array = {values...};
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
    auto retrieveAllArgs = [&f](auto &&...xs) { (static_cast<F &&>(f)(FWD(xs)), ...); };
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
