#pragma once
#include "../lpl/lpl.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <iterator>
#include <numeric>
#include <optional>
#include <ratio>
#include <tuple>

#define decltype_t(t) typename std::decay_t<decltype(t)>::type

#define typed_static_assert(f) static_assert(decltype(f){})
#define typed_static_assert_msg(f, msg) static_assert(decltype(f){}, msg)
#define if_constexpr(c) if constexpr (decltype(c){})
#define else_if_constexpr(c) else if constexpr (decltype(c){})

#define LTL_REQUIRE(b) typename = std::enable_if_t<(b)>
#define LTL_REQUIRE_T(b) std::enable_if_t<decltype(b){}, bool> = true

namespace ltl {
/////////////////////// FWD
#define FWD(x) ::std::forward<decltype(x)>(x)

/////////////////////// decay_reference_wrapper
template <typename T> struct decay_reference_wrapper { using type = std::decay_t<T>; };

template <typename T> struct decay_reference_wrapper<std::reference_wrapper<T>> {
  using type = T &;
};

template <typename T>
using decay_reference_wrapper_t = typename decay_reference_wrapper<T>::type;

///////////////////// overloader
template <typename... Fs> struct overloader : Fs... {
  constexpr overloader(Fs &&... fs) : Fs{std::move(fs)}... {}

  using Fs::operator()...;
};

template <typename... Fs> overloader(Fs...)->overloader<Fs...>;

////////////////////// crtp
template <typename T, template <typename> typename crtpType> struct crtp {
  T &underlying() { return static_cast<T &>(*this); }

  constexpr const T &underlying() const { return static_cast<const T &>(*this); }
};

///////////////////// bool
template <bool v> struct bool_t {
  static constexpr auto value = v;
  constexpr operator bool() const { return v; }
};

using false_t = bool_t<false>;
using true_t = bool_t<true>;
} // namespace ltl

constexpr ltl::false_t false_v;
constexpr ltl::true_t true_v;

namespace ltl {
template <bool v> constexpr bool_t<v> bool_v{};

#define OP(op)                                                                           \
  template <bool v1, bool v2>                                                            \
  [[nodiscard]] constexpr bool_t<v1 op v2> operator op(bool_t<v1>, bool_t<v2>) {         \
    return {};                                                                           \
  }

LPL_MAP(OP, ==, !=, &&, ||)

#undef OP

template <bool v>[[nodiscard]] constexpr bool_t<!v> operator!(bool_t<v>) { return {}; }

///////////////////////// type
template <typename T> struct type_t {
  using type = T;
  constexpr type_t() = default;
};

template <typename T> constexpr type_t<T> type_v{};

template <typename T1, typename T2>
[[nodiscard]] constexpr false_t operator==(type_t<T1>, type_t<T2>) {
  return {};
}

template <typename T>[[nodiscard]] constexpr true_t operator==(type_t<T>, type_t<T>) {
  return {};
}

template <typename T1, typename T2>
[[nodiscard]] constexpr true_t operator!=(type_t<T1>, type_t<T2>) {
  return {};
}

template <typename T>[[nodiscard]] constexpr false_t operator!=(type_t<T>, type_t<T>) {
  return {};
}

////////////////////// number
template <int N> struct number_t { constexpr static int value = N; };

namespace detail {
template <char... _digits>[[nodiscard]] constexpr int digits_to_int() {
  constexpr char digits[] = {_digits...};
  int result = 0;
  for (int digit : digits) {
    result *= 10;
    result += digit - '0';
  }

  return result;
}
} // namespace detail
} // namespace ltl

template <int N> constexpr ltl::number_t<N> number_v{};
template <char... digits>[[nodiscard]] constexpr auto operator""_n() {
  return number_v<ltl::detail::digits_to_int<digits...>()>;
}

namespace ltl {

#define OP(op)                                                                           \
  template <int N1, int N2>                                                              \
  [[nodiscard]] constexpr number_t<(N1 op N2)> operator op(number_t<N1>, number_t<N2>) { \
    return {};                                                                           \
  }                                                                                      \
  template <int N, bool v>                                                               \
  [[nodiscard]] constexpr number_t<(N op static_cast<int>(v))> operator op(number_t<N>,  \
                                                                           bool_t<v>) {  \
    return {};                                                                           \
  }                                                                                      \
  template <int N, bool v>                                                               \
  [[nodiscard]] constexpr number_t<(static_cast<int>(v) op N)> operator op(              \
      bool_t<v>, number_t<N>) {                                                          \
    return {};                                                                           \
  }
LPL_MAP(OP, +, -, *, /, %, &, |, ^, <<, >>)
#undef OP

#define OP(op)                                                                           \
  template <int N1, int N2>                                                              \
  [[nodiscard]] constexpr bool_t<(N1 op N2)> operator op(number_t<N1>, number_t<N2>) {   \
    return {};                                                                           \
  }

LPL_MAP(OP, ==, !=, >, >=, <, <=)
#undef OP

#define OP(op)                                                                           \
  template <int N>[[nodiscard]] constexpr number_t<(op N)> operator op(number_t<N>) {    \
    return {};                                                                           \
  }
LPL_MAP(OP, ~, +, -)
#undef OP

template <typename T>[[nodiscard]] constexpr auto max(T a) { return a; }

template <typename T1, typename T2, typename... Ts>
[[nodiscard]] constexpr auto max(T1 a, T2 b, Ts... ts) {
  if_constexpr(a > b) return ::ltl::max(a, ts...);
  else return ::ltl::max(b, ts...);
}

template <typename T>[[nodiscard]] constexpr auto min(T a) { return a; }
template <typename T1, typename T2, typename... Ts>
[[nodiscard]] constexpr auto min(T1 a, T2 b, Ts... ts) {
  if_constexpr(a < b) return ::ltl::min(a, ts...);
  else return ::ltl::min(b, ts...);
}

///////////////////////// is_valid
namespace detail {
template <typename F, typename... Fs>
constexpr auto testValidity(F &&f, Fs &&... fs) -> decltype(FWD(f)(FWD(fs)...), true_v);

constexpr false_t testValidity(...);
} // namespace detail
template <typename F> constexpr auto is_valid(F f) {
  return [f](auto &&... x) -> decltype(detail::testValidity(f, FWD(x)...)) {
    (void)f;
    return {};
  };
}

#define LTL_WRITE_AUTO_WITH_COMMA_IMPL(x) , auto x

#define LTL_WRITE_AUTO_IMPL(x, ...)                                                      \
  (auto x LPL_MAP(LTL_WRITE_AUTO_WITH_COMMA_IMPL, __VA_ARGS__))

#define LTL_ENSURE_NOT_EMPTY(...) (__VA_ARGS__, ())
#define IS_VALID(variables, expr)                                                        \
  ltl::is_valid([] LPL_IDENTITY(LTL_WRITE_AUTO_IMPL LTL_ENSURE_NOT_EMPTY variables)      \
                    -> decltype(expr, void()) {})

/////////////////////////// Tuple
namespace detail {
template <typename... Ts> struct tuple_applied {
  template <bool isNotEmpty = (sizeof...(Ts) > 0), LTL_REQUIRE(isNotEmpty)>
  constexpr tuple_applied() : m_tuple{Ts{}...} {}

  constexpr tuple_applied(Ts &&... ts) : m_tuple{FWD(ts)...} {}

  template <typename F> decltype(auto) operator()(F &&f) & {
    return std::apply(FWD(f), m_tuple);
  }

  template <typename F> constexpr decltype(auto) operator()(F &&f) const & {
    return std::apply(FWD(f), m_tuple);
  }

  template <typename F> constexpr decltype(auto) operator()(F &&f) && {
    return std::apply(FWD(f), std::move(m_tuple));
  }

  std::tuple<Ts...> m_tuple;
};

} // namespace detail

template <typename... Ts> class tuple_t {
public:
  constexpr static auto length = number_v<sizeof...(Ts)>;
  constexpr static auto isEmpty = length == 0_n;

  template <bool isNotEmpty = !isEmpty, LTL_REQUIRE(isNotEmpty)>
  explicit constexpr tuple_t() : m_storage{} {}

  explicit constexpr tuple_t(Ts... ts) : m_storage{FWD(ts)...} {}

  template <typename F>
      decltype(auto) operator()(F &&f) &
      noexcept(noexcept(std::declval<F>()(std::declval<Ts &>()...))) {
    return m_storage(FWD(f));
  }

  template <typename F>
  constexpr decltype(auto) operator()(F &&f) const &noexcept(
      noexcept(std::declval<F>()(std::declval<const Ts &>()...))) {
    return m_storage(FWD(f));
  }

  template <typename F>
      constexpr decltype(auto) operator()(F &&f) &&
      noexcept(noexcept(std::declval<F>()(std::declval<Ts>()...))) {
    return std::move(m_storage)(FWD(f));
  }

  template <int N>[[nodiscard]] auto &get(number_t<N> n) & noexcept {
    typed_static_assert(n < length);
    return std::get<N>(m_storage.m_tuple);
  }

  template <int N>[[nodiscard]] constexpr const auto &get(number_t<N> n) const &noexcept {
    typed_static_assert(n < length);
    return std::get<N>(m_storage.m_tuple);
  }

  template <int N>[[nodiscard]] constexpr auto &&get(number_t<N> n) && noexcept {
    typed_static_assert(n < length);
    return std::get<N>(std::move(m_storage.m_tuple));
  }

  template <int N>[[nodiscard]] auto &operator[](number_t<N> n) & noexcept {
    return get(n);
  }

  template <int N>
  [[nodiscard]] constexpr const auto &operator[](number_t<N> n) const &noexcept {
    return get(n);
  }

  template <int N>[[nodiscard]] constexpr auto &&operator[](number_t<N> n) && noexcept {
    return std::move(*this).get(n);
  }

  template <int... Is>
  [[nodiscard]] constexpr auto extract(number_t<Is>... ns) const &noexcept {
    constexpr tuple_t<type_t<Ts>...> types{};
    return tuple_t<decltype_t(types[ns])...>{get(ns)...};
  }

  template <int... Is>
      [[nodiscard]] constexpr auto extract(number_t<Is>... ns) && noexcept {
    constexpr tuple_t<type_t<Ts>...> types{};
    return tuple_t<decltype_t(types[ns])...>{std::move(*this).get(ns)...};
  }

  template <typename... _Ts,
            LTL_REQUIRE(type_v<tuple_t<_Ts...>> != type_v<tuple_t<Ts...>>)>
  [[nodiscard]] constexpr bool operator==(const tuple_t<_Ts...> &) const noexcept {
    return false;
  }

  template <typename... _Ts,
            LTL_REQUIRE(type_v<tuple_t<_Ts...>> != type_v<tuple_t<Ts...>>)>
  [[nodiscard]] constexpr bool operator!=(const tuple_t<_Ts...> &) const noexcept {
    return true;
  }

  [[nodiscard]] constexpr bool operator==(const tuple_t<Ts...> &t) const noexcept {
    return t.m_storage.m_tuple == m_storage.m_tuple;
  }

  [[nodiscard]] constexpr bool operator!=(const tuple_t<Ts...> &t) const noexcept {
    return t.m_storage.m_tuple != m_storage.m_tuple;
  }

  template <typename T>[[nodiscard]] constexpr auto push_back(T &&newValue) const & {
    auto fwdAll = [&newValue](const auto &... xs) {
      return tuple_t<Ts..., decay_reference_wrapper_t<T>>{xs..., FWD(newValue)};
    };
    return m_storage(fwdAll);
  }

  template <typename T>[[nodiscard]] constexpr auto push_back(T &&newValue) && {
    auto fwdAll = [&newValue](Ts &&... xs) {
      return tuple_t<Ts..., decay_reference_wrapper_t<T>>{FWD(xs)..., FWD(newValue)};
    };
    return std::move(m_storage)(fwdAll);
  }

  template <typename T>[[nodiscard]] constexpr auto push_front(T &&newValue) const & {
    auto fwdAll = [&newValue](const auto &... xs) {
      return tuple_t<decay_reference_wrapper_t<T>, Ts...>{FWD(newValue), xs...};
    };
    return m_storage(fwdAll);
  }

  template <typename T>[[nodiscard]] constexpr auto push_front(T &&newValue) && {
    auto fwdAll = [&newValue](Ts &&... xs) {
      return tuple_t<decay_reference_wrapper_t<T>, Ts...>{FWD(newValue), FWD(xs)...};
    };
    return std::move(m_storage)(fwdAll);
  }

  [[nodiscard]] constexpr auto pop_back() const & {
    auto extracter = [this](auto... numbers) { return this->extract(numbers...); };
    constexpr auto numbers = build_index_sequence(length - 1_n);
    return numbers(extracter);
  }

  [[nodiscard]] constexpr auto pop_back() && {
    auto extracter = [this](auto... numbers) {
      return std::move(*this).extract(numbers...);
    };
    constexpr auto numbers = build_index_sequence(length - 1_n);
    return numbers(extracter);
  }

  [[nodiscard]] constexpr auto pop_front() const & {
    auto extracter = [this](auto... numbers) { return this->extract(numbers...); };
    constexpr auto numbers = build_index_sequence(1_n, length);
    return apply(numbers, extracter);
  }

  [[nodiscard]] constexpr auto pop_front() && {
    auto extracter = [this](auto... numbers) {
      return std::move(*this).extract(numbers...);
    };
    constexpr auto numbers = build_index_sequence(1_n, length);
    return apply(numbers, extracter);
  }

  template <typename N1, typename N2>
  [[nodiscard]] static constexpr auto build_index_sequence(N1 n1, N2 n2) {
    return build_index_sequence_helper(n1, n2);
  }

  template <typename N>[[nodiscard]] static constexpr auto build_index_sequence(N n) {
    return build_index_sequence(0_n, n);
  }

private:
  template <int N1, int N2, typename List = tuple_t<>>
  [[nodiscard]] static constexpr auto
  build_index_sequence_helper(number_t<N1> n1, number_t<N2> n2,
                              const List list = List{}) {
    typed_static_assert_msg(n1 <= n2, "n1 must be lesser or equal to n2");
    if constexpr (n1 == n2) {
      return list;
    }

    else {
      return build_index_sequence_helper(n1 + 1_n, n2, list.push_back(n1));
    }
  }

private:
  detail::tuple_applied<Ts...> m_storage;
};

template <typename... Ts> tuple_t(Ts...)->tuple_t<decay_reference_wrapper_t<Ts>...>;

////////////////////// Templates
/// Convenience types
template <typename... Types> using type_list_t = tuple_t<type_t<Types>...>;
template <int... Ns> using number_list_t = tuple_t<number_t<Ns>...>;
template <bool... Bs> using bool_list_t = tuple_t<bool_t<Bs>...>;

// Convenience variables
template <typename... Types> constexpr type_list_t<Types...> type_list_v{};
template <int... Ns> constexpr number_list_t<Ns...> number_list_v{};
template <bool... Bs> constexpr bool_list_t<Bs...> bool_list_v{};

// Conditional functions
template <typename T>[[nodiscard]] constexpr false_t is_tuple_t(T) { return {}; }
template <typename... Ts>[[nodiscard]] constexpr true_t is_tuple_t(tuple_t<Ts...>) {
  return {};
}

template <typename T>[[nodiscard]] constexpr false_t is_type_list_t(T) { return {}; }
template <typename... Ts>
[[nodiscard]] constexpr true_t is_type_list_t(type_list_t<Ts...>) {
  return {};
}

template <typename T>[[nodiscard]] constexpr false_t is_number_list_t(T) { return {}; }
template <int... Ns>
[[nodiscard]] constexpr true_t is_number_list_t(number_list_t<Ns...>) {
  return {};
}

template <typename T>[[nodiscard]] constexpr false_t is_bool_list_t(T) { return {}; }
template <bool... Bs>[[nodiscard]] constexpr true_t is_bool_list_t(bool_list_t<Bs...>) {
  return {};
}

template <typename T>[[nodiscard]] constexpr false_t is_number_t(T) { return {}; }
template <int N>[[nodiscard]] constexpr true_t is_number_t(number_t<N>) { return {}; }

template <typename T>[[nodiscard]] constexpr false_t is_bool_t(T) { return {}; }
template <bool B>[[nodiscard]] constexpr true_t is_bool_t(bool_t<B>) { return {}; }

#define IS_TYPE(name)                                                                    \
  template <typename T>                                                                  \
  [[nodiscard]] constexpr decltype(name(std::declval<T>())) name(type_t<T>) {            \
    return {};                                                                           \
  }

IS_TYPE(is_bool_t)
IS_TYPE(is_bool_list_t)
IS_TYPE(is_number_t)
IS_TYPE(is_number_list_t)
IS_TYPE(is_type_list_t)
IS_TYPE(is_tuple_t)
#undef IS_TYPE

/////////////////////// Arguments
template <typename F, typename Tuple, LTL_REQUIRE(is_tuple_t(type_v<Tuple>))>
constexpr decltype(auto) apply(Tuple &&tuple,
                               F &&f) noexcept(noexcept(FWD(tuple)(FWD(f)))) {
  typed_static_assert(is_tuple_t(tuple));
  return FWD(tuple)(FWD(f));
}

template <typename F, typename Tuple, LTL_REQUIRE(is_tuple_t(type_v<Tuple>))>
F &&for_each(Tuple &&tuple, F &&f) {
  typed_static_assert(is_tuple_t(tuple));

  auto retrieveAllArgs = [&f](auto &&... xs) { (FWD(f)(FWD(xs)), ...); };
  ltl::apply(FWD(tuple), retrieveAllArgs);
  return FWD(f);
}

////////////////////// Algorithm tuple
template <typename N1, typename N2>
[[nodiscard]] constexpr auto build_index_sequence(N1 n1, N2 n2) {
  return tuple_t<>::build_index_sequence(n1, n2);
}

template <typename N>[[nodiscard]] constexpr auto build_index_sequence(N n) {
  return tuple_t<>::build_index_sequence(0_n, n); // does not compile
}

template <typename... Ts, typename T>
constexpr auto contains_type(const tuple_t<Ts...> &tuple, type_t<T> type) {
  if_constexpr(is_type_list_t(tuple)) return (false_v || ... || (Ts{} == type));
  else return contains_type(type_list_v<Ts...>, type);
}

template <typename... Ts, typename T>
constexpr auto count_type(const tuple_t<Ts...> &tuple, type_t<T> type) {
  if_constexpr(is_type_list_t(tuple)) return (0_n + ... + (Ts{} == type));
  else return count_type(type_list_v<Ts...>, type);
}

template <typename... Ts, typename T, int N = 0>
constexpr auto find_type(const tuple_t<Ts...> &tuple, type_t<T> type,
                         number_t<N> first = {}) {
  if_constexpr(is_type_list_t(tuple)) {
    if_constexpr(tuple[first] == type) return first;
    else return find_type(tuple, type, first + 1_n);
  }

  else return find_type(type_list_v<Ts...>, type, first);
}

template <typename... Ts, typename P, int N = 0>
constexpr auto find_if_type(const tuple_t<Ts...> &tuple, P &&p, number_t<N> first = {}) {
  if_constexpr(is_type_list_t(tuple)) {
    if_constexpr(FWD(p)(tuple[first])) return first;
    else return find_if_type(tuple, FWD(p), first + 1_n);
  }
  else return find_if_type(type_list_v<Ts...>, FWD(p), first);
}

template <typename... Ts, typename P>
constexpr auto contains_if_type(const tuple_t<Ts...> &tuple, P &&p) {
  if_constexpr(is_type_list_t(tuple)) return (false_v || ... || (FWD(p)(Ts{})));
  else return contains_type(type_list_v<Ts...>, FWD(p));
}

template <typename... Ts, typename P>
constexpr auto count_if_type(const tuple_t<Ts...> &tuple, P &&p) {
  if_constexpr(is_type_list_t(tuple)) return (0_n + ... + (FWD(p)(Ts{})));
  else return count_type(type_list_v<Ts...>, FWD(p));
}

template <typename... Ts, typename P>
constexpr auto all_of_type(const tuple_t<Ts...> &tuple, P &&p) {
  if_constexpr(is_type_list_t(tuple)) return (true_v && ... && (FWD(p)(Ts{})));
  else return all_of_type(type_list_v<Ts...>, FWD(p));
}

template <typename... Ts, typename P>
constexpr auto any_of_type(const tuple_t<Ts...> &tuple, P &&p) {
  if_constexpr(is_type_list_t(tuple)) return (false_v || ... || (FWD(p)(Ts{})));
  else return any_of_type(type_list_v<Ts...>, FWD(p));
}

template <typename... Ts, typename P>
constexpr auto none_of_type(const tuple_t<Ts...> &tuple, P &&p) {
  return !any_of_type(tuple, FWD(p));
}

/////////////////////////// Traits
#define TRAIT(name)                                                                      \
  template <typename T>                                                                  \
      [[nodiscard]] constexpr bool_t<std::LPL_CAT(name, _v) < T>>                        \
      LPL_CAT(name, _Impl)(type_t<T>) {                                                  \
    return {};                                                                           \
  }                                                                                      \
  constexpr auto name = [](auto x) constexpr { return LPL_CAT(name, _Impl)(x); };

#define TRAIT_ARG(name)                                                                  \
  template <typename T, typename Arg>                                                    \
      [[nodiscard]] constexpr bool_t<std::LPL_CAT(name, _v) < T, Arg>>                   \
      name(type_t<T>, type_t<Arg>) {                                                     \
    return {};                                                                           \
  }                                                                                      \
                                                                                         \
  template <typename T, typename Arg>                                                    \
      [[nodiscard]] constexpr bool_t<std::LPL_CAT(name, _v) < T, Arg>>                   \
      name(type_t<T>, type_list_t<Arg>) {                                                \
    return {};                                                                           \
  }

#define TRAIT_ARGS(name)                                                                 \
  template <typename T, typename... Args>                                                \
      [[nodiscard]] constexpr bool_t<std::LPL_CAT(name, _v) < T, Args...>>               \
      name(type_t<T>, type_t<Args>...) {                                                 \
    return {};                                                                           \
  }                                                                                      \
                                                                                         \
  template <typename T, typename... Args>                                                \
      [[nodiscard]] constexpr bool_t<std::LPL_CAT(name, _v) < T, Args...>>               \
      name(type_t<T>, type_list_t<Args...>) {                                            \
    return {};                                                                           \
  }

// Primary type categories
LPL_MAP(TRAIT, is_void, is_null_pointer, is_integral, is_floating_point, is_array,
        is_enum, is_union, is_class, is_function, is_pointer, is_lvalue_reference,
        is_rvalue_reference, is_member_object_pointer, is_member_function_pointer)

// Composite type categories
LPL_MAP(TRAIT, is_fundamental, is_arithmetic, is_scalar, is_object, is_compound,
        is_reference, is_member_pointer)

// Type properties
LPL_MAP(TRAIT, is_const, is_volatile, is_trivial, is_trivially_copyable,
        is_standard_layout, has_unique_object_representations, is_empty, is_polymorphic,
        is_abstract, is_final, is_aggregate, is_signed, is_unsigned)

// Supported operations
LPL_MAP(TRAIT, is_default_constructible, is_trivially_default_constructible,
        is_nothrow_default_constructible, is_copy_constructible,
        is_trivially_copy_constructible, is_nothrow_copy_constructible,
        is_move_constructible, is_trivially_move_constructible,
        is_nothrow_move_constructible)

LPL_MAP(TRAIT, is_copy_assignable, is_trivially_copy_assignable,
        is_nothrow_copy_assignable, is_move_assignable, is_trivially_move_assignable,
        is_nothrow_move_assignable)

LPL_MAP(TRAIT_ARG, is_assignable, is_trivially_assignable, is_nothrow_assignable,
        is_swappable_with, is_nothrow_swappable_with)

LPL_MAP(TRAIT, is_destructible, is_trivially_destructible, is_nothrow_destructible,
        has_virtual_destructor, is_swappable, is_nothrow_swappable)

LPL_MAP(TRAIT_ARGS, is_constructible, is_trivially_constructible,
        is_nothrow_constructible)

// Type relationships
LPL_MAP(TRAIT_ARG, is_same, is_base_of, is_convertible)

LPL_MAP(TRAIT_ARGS, is_invocable, is_invocable_r, is_nothrow_invocable,
        is_nothrow_invocable_r)

#undef TRAIT
#undef TRAIT_ARG
#undef TRAIT_ARGS

#define TRAIT(name)                                                                      \
  template <typename T>                                                                  \
      [[nodiscard]] constexpr number_t<std::LPL_CAT(name, _v) < T>> name(type_t<T>) {    \
    return {};                                                                           \
  }

// Property queries
template <typename T, int N = 0>
[[nodiscard]] constexpr number_t<std::extent_v<T, N>>
extent(type_t<T>, number_t<N> = number_t<N>{}) {
  return {};
}

LPL_MAP(TRAIT, alignment_of, rank)

#undef TRAIT

#define TRAIT(name)                                                                      \
  template <typename T>                                                                  \
      [[nodiscard]] constexpr type_t<std::LPL_CAT(name, _t) < T>> name(type_t<T>) {      \
    return {};                                                                           \
  }

// const-volatibility specifiers
LPL_MAP(TRAIT, remove_cv, remove_const, remove_volatile, add_cv, add_const, add_volatile)

// references
LPL_MAP(TRAIT, remove_reference, add_lvalue_reference, add_rvalue_reference)

// pointers
LPL_MAP(TRAIT, remove_pointer, add_pointer)

// Sign modifiers
LPL_MAP(TRAIT, make_signed, make_unsigned)

// Arrays
LPL_MAP(TRAIT, remove_extent, remove_all_extents)
#undef TRAIT

template <typename T>[[nodiscard]] constexpr auto is_iterable(type_t<T>) {
  constexpr auto trait = IS_VALID((x), (std::begin(x), std::end(x)));
  return decltype(trait(std::declval<T>())){};
}

template <typename T>[[nodiscard]] constexpr auto is_iterable(T) {
  return is_iterable(type_v<T>);
}

#define LTL_MAKE_IS_KIND(type, name)                                                     \
  template <typename T>                                                                  \
  [[nodiscard]] constexpr ::ltl::false_t LPL_CAT(name, Impl)(::ltl::type_t<T>) {         \
    return {};                                                                           \
  }                                                                                      \
  template <typename... Ts>                                                              \
  [[nodiscard]] constexpr ::ltl::true_t LPL_CAT(name,                                    \
                                                Impl)(::ltl::type_t<type<Ts...>>) {      \
    return {};                                                                           \
  }                                                                                      \
  template <typename T>[[nodiscard]] constexpr auto LPL_CAT(name, Impl)(T &&) {          \
    return LPL_CAT(name, Impl)(::ltl::type_v<::std::decay_t<T>>);                        \
  }                                                                                      \
  constexpr auto name = [](auto &&x) constexpr { return LPL_CAT(name, Impl)(FWD(x)); }

template <typename T> constexpr auto is_type(type_t<T> type) {
  return [type](auto x) constexpr { return type == x; };
}

template <typename T> constexpr auto is_derived_from(type_t<T> type) {
  return [type](auto x) constexpr { return is_base_of(type, x); };
}

////////////// StrongTypes
struct ConverterIdentity {
  template <typename T>[[nodiscard]] static constexpr T convertToReference(const T &v) {
    return v;
  }

  template <typename T>[[nodiscard]] static constexpr T convertFromReference(const T &v) {
    return v;
  }
};
namespace detail {

template <typename T, typename Tag, typename Converter,
          template <typename...> typename... Skills>
class strong_type_t : public Skills<strong_type_t<T, Tag, Converter, Skills...>>... {
  static constexpr bool isDefaultConstructible = is_default_constructible(type_v<T>);

  template <typename> struct isSameKindTrait : false_t {};
  template <typename C>
  struct isSameKindTrait<strong_type_t<T, Tag, C, Skills...>> : true_t {};

  template <typename U>
  static constexpr bool isSameKind_v = isSameKindTrait<std::decay_t<U>>{};

public:
  template <typename U>
  [[nodiscard]] static isSameKindTrait<std::decay_t<U>> isSameKind(U);

  template <bool dc = isDefaultConstructible, LTL_REQUIRE(dc)>
  explicit constexpr strong_type_t() : m_value{} {}

  template <typename... Args, LTL_REQUIRE(sizeof...(Args) > 0),
            LTL_REQUIRE(!isSameKind_v<Args> && ...)>
  explicit constexpr strong_type_t(Args &&... args) : m_value{FWD(args)...} {}

  [[nodiscard]] T &get() & { return m_value; }
  [[nodiscard]] constexpr const T &get() const & { return m_value; }
  [[nodiscard]] constexpr T &&get() && { return std::move(m_value); }

  template <typename OtherConverter,
            LTL_REQUIRE(ltl::type_v<OtherConverter> != ltl::type_v<Converter>)>
  [[nodiscard]] constexpr
  operator strong_type_t<T, Tag, OtherConverter, Skills...>() const {
    return strong_type_t<T, Tag, OtherConverter, Skills...>{
        OtherConverter::convertFromReference(Converter::convertToReference(m_value))};
  }

private:
  T m_value;
}; // namespace detail
} // namespace detail

template <typename T, typename Tag, template <typename...> typename... Skills>
using strong_type_t = detail::strong_type_t<T, Tag, ConverterIdentity, Skills...>;

////////////////// Arithmetic strong types
#define OP(name, op)                                                                     \
  template <typename T> struct name : crtp<T, name> {                                    \
    T &operator LPL_CAT(op, =)(const T &other) {                                         \
      this->underlying().get() LPL_CAT(op, =) other.get();                               \
      return *this;                                                                      \
    }                                                                                    \
                                                                                         \
    [[nodiscard]] constexpr friend T operator op(const T &a, const T &b) {               \
      return T{a.get() op b.get()};                                                      \
    }                                                                                    \
                                                                                         \
    template <typename T2> T &operator LPL_CAT(op, =)(const T2 &other) {                 \
      typed_static_assert(this->underlying().isSameKind(other));                         \
      this->underlying().get() LPL_CAT(op, =) static_cast<T>(other).get();               \
      return *this;                                                                      \
    }                                                                                    \
                                                                                         \
    template <typename T2>                                                               \
    [[nodiscard]] constexpr friend T operator op(const T &a, const T2 &b) {              \
      typed_static_assert(a.isSameKind(b));                                              \
      return T{a.get() op static_cast<T>(b).get()};                                      \
    }                                                                                    \
  };

OP(Addable, +)
OP(Subtractable, -)
OP(Multipliable, *)
OP(Dividable, /)
OP(Moduloable, %)

#undef OP

#define OP(name, op)                                                                     \
  template <typename T> struct name : crtp<T, name> {                                    \
    constexpr T &operator op() {                                                         \
      op this->underlying().get();                                                       \
      return this->underlying();                                                         \
    }                                                                                    \
                                                                                         \
    constexpr T operator op(int) {                                                       \
      this->underlying().get() op;                                                       \
      return *this->underlying();                                                        \
    }                                                                                    \
  };

OP(Incrementable, ++)
OP(Decrementable, --)

#undef OP

/////////////////// StrongType Converter
template <typename Ratio> struct ConverterMultiplier {
  template <typename T>[[nodiscard]] static constexpr T convertToReference(const T &v) {
    return v * Ratio::num / Ratio::den;
  }

  template <typename T>[[nodiscard]] static constexpr T convertFromReference(const T &v) {
    return v * Ratio::den / Ratio::num;
  }
};

namespace detail {
template <typename Base, typename Converter> struct AddConverter;

template <typename T, typename Tag, typename Converter,
          template <typename...> typename... Skills>
struct AddConverter<strong_type_t<T, Tag, ConverterIdentity, Skills...>, Converter> {
  using type = strong_type_t<T, Tag, Converter, Skills...>;
};

template <typename Base, typename Ratio> struct MultipleOf;

template <typename T, typename Tag, typename Ratio,
          template <typename...> typename... Skills>
struct MultipleOf<strong_type_t<T, Tag, ConverterIdentity, Skills...>, Ratio> {
  using type = strong_type_t<T, Tag, ConverterMultiplier<Ratio>, Skills...>;
};

template <typename T, typename Tag, typename RatioBase, typename RatioToApply,
          template <typename...> typename... Skills>
struct MultipleOf<strong_type_t<T, Tag, ConverterMultiplier<RatioBase>, Skills...>,
                  RatioToApply> {
  using type =
      strong_type_t<T, Tag,
                    ConverterMultiplier<std::ratio<RatioBase::num * RatioToApply::num,
                                                   RatioBase::den * RatioToApply::den>>,
                    Skills...>;
};

} // namespace detail

template <typename Base, typename Converter>
using add_converter = typename detail::AddConverter<Base, Converter>::type;

template <typename Base, typename Ratio>
using multiple_of = typename detail::MultipleOf<Base, Ratio>::type;

/////////////// Equality for strong types
#define OP(name, op)                                                                     \
  template <typename T> struct name {                                                    \
    [[nodiscard]] constexpr friend bool operator op(const T &a, const T &b) {            \
      return a.get() op b.get();                                                         \
    }                                                                                    \
                                                                                         \
    template <typename T2>                                                               \
    [[nodiscard]] constexpr friend bool operator op(const T &a, const T2 &b) {           \
      typed_static_assert(a.isSameKind(b));                                              \
      return a.get() op static_cast<T>(b).get();                                         \
    }                                                                                    \
  };

OP(GreaterThan, >)
OP(LessThan, <)
OP(GreaterThanEqual, >=)
OP(LessThanEqual, <=)

namespace detail {
OP(LTLSTEquality, ==)
OP(LTLSTInequality, !=)
} // namespace detail

template <typename T>
struct EqualityComparable : detail::LTLSTEquality<T>, detail::LTLSTInequality<T> {};

template <typename T>
struct AllComparable : EqualityComparable<T>,
                       GreaterThan<T>,
                       LessThan<T>,
                       GreaterThanEqual<T>,
                       LessThanEqual<T> {};

#undef OP

//////////////////// Stream for strong types
template <typename T> struct OStreamable {
  friend std::ostream &operator<<(std::ostream &s, const T &v) { return s << v.get(); }
};

////////////////////// Smart iterators
namespace detail {
using Index = ltl::strong_type_t<std::size_t, struct IndexTag, Incrementable>;

#define DEFINE_ITERATOR(name)                                                            \
  using difference_type = typename std::iterator_traits<name>::difference_type;          \
  using pointer = typename std::iterator_traits<name>::pointer;                          \
  using iterator_category = std::input_iterator_tag

template <typename It> class enumerate_range {
  struct enumerate_iterator {
    DEFINE_ITERATOR(It);
    using value_type =
        std::pair<const std::size_t, typename std::iterator_traits<It>::reference>;
    using reference = value_type;

    constexpr enumerate_iterator(It &&it) : index{std::size_t{0}}, it{std::move(it)} {}

    constexpr enumerate_iterator &operator++() {
      ++it;
      ++index;
      return *this;
    }

    constexpr auto operator*() { return value_type{index.get(), *it}; }

    constexpr bool operator!=(const enumerate_iterator &it2) const {
      return it != it2.it;
    }

    Index index;
    It it;
  };

public:
  constexpr enumerate_range(It &&begin, It &&end)
      : m_begin{std::move(begin)}, m_end{std::move(end)} {}

  [[nodiscard]] constexpr auto begin() { return m_begin; }
  [[nodiscard]] constexpr auto end() { return m_end; }

private:
  enumerate_iterator m_begin;
  enumerate_iterator m_end;
};

template <typename It, typename Filter> class filter_range {
  struct filter_iterator {
    DEFINE_ITERATOR(It);
    using value_type = typename std::iterator_traits<It>::value_type;
    using reference = typename std::iterator_traits<It>::reference;

    constexpr filter_iterator() = default;

    constexpr filter_iterator(It &&it, It &&end, Filter &&filter)
        : it{std::move(it)}, end{std::move(end)}, filter{std::move(filter)} {
      advance();
    }

    constexpr filter_iterator &operator++() {
      assert(it);
      ++(*it);
      advance();
      return *this;
    }

    constexpr reference operator*() { return **it; }

    constexpr void advance() {
      assert(it && end && filter);
      while (*it != *end && (*filter)(**it) == false)
        ++(*it);
    }

    constexpr bool operator!=(const filter_iterator &it2) const {
      if (it && it2.it)
        return *it != *it2.it;
      assert(it && end);
      return *it != *end;
    }

    std::optional<It> it;
    std::optional<It> end;
    std::optional<Filter> filter;
  };

public:
  constexpr filter_range(It &&begin, It &&end, Filter filter)
      : m_begin{std::move(begin), std::move(end), std::move(filter)} {}

  constexpr auto begin() { return m_begin; }
  constexpr auto end() { return m_end; }

private:
  filter_iterator m_begin;
  filter_iterator m_end;
};

template <typename It, typename MapFunction> class map_range {
  struct map_iterator {
    DEFINE_ITERATOR(It);
    using value_type = decltype(std::declval<MapFunction>()(
        std::declval<typename std::iterator_traits<It>::reference>()));
    using reference = value_type;

    constexpr map_iterator(It &&it) : it{std::move(it)} {}

    constexpr map_iterator(It &&it, MapFunction &&mapFunction)
        : it{std::move(it)}, mapFunction{std::move(mapFunction)} {}

    constexpr auto operator*() {
      assert(mapFunction);
      return (*mapFunction)(*it);
    }

    constexpr map_iterator &operator++() {
      ++it;
      return *this;
    }

    constexpr bool operator!=(const map_iterator &it2) const { return it != it2.it; }

    It it;
    std::optional<MapFunction> mapFunction;
  };

public:
  constexpr map_range(It &&begin, It &&end, MapFunction mapFunction)
      : m_begin{std::move(begin), std::move(mapFunction)}, m_end{std::move(end)} {}

  constexpr auto begin() { return m_begin; }
  constexpr auto end() { return m_end; }

private:
  map_iterator m_begin;
  map_iterator m_end;
};

template <typename... Containers> class zip_range {
  type_list_t<Containers...> container_types;

  typed_static_assert_msg(container_types.length > 0_n, "Containers must not be empty");

  struct zip_range_iterator {
    using difference_type = int;
    using value_type = std::tuple<decltype(*std::declval<Containers>().begin())...>;
    using pointer = value_type;
    using reference = value_type;
    using iterator_category = std::input_iterator_tag;

    template <typename... Its>
    constexpr zip_range_iterator(Its &&... iterators)
        : m_iterators{std::forward<Its>(iterators)...} {}

    constexpr auto operator*() {
      auto foo = [](auto &... xs) { return value_type(*xs...); };
      return m_iterators(foo);
    }

    constexpr auto operator!=(const zip_range_iterator &other) {
      return m_iterators[0_n] != other.m_iterators[0_n];
    }

    constexpr zip_range_iterator &operator++() {
      m_iterators([](auto &... xs) { (xs++, ...); });
      return *this;
    }

    ltl::tuple_t<decltype(std::declval<Containers>().begin())...> m_iterators;
  };

public:
  constexpr zip_range(Containers... containers)
      : m_containers{std::forward<Containers>(containers)...} {
    m_containers([](auto &c0, auto &... cs) {
      assert((true && ... && (c0.size() == cs.size())));
    });
  }

  constexpr auto begin() {
    return m_containers([](auto &... cs) { return zip_range_iterator{cs.begin()...}; });
  }

  constexpr auto end() {
    return m_containers([](auto &... cs) { return zip_range_iterator{cs.end()...}; });
  }

private:
  tuple_t<Containers...> m_containers;
};

template <typename Container> class sorted_inserter_iterator {
public:
  using difference_type = void;
  using value_type = void;
  using pointer = void;
  using reference = void;
  using iterator_category = std::output_iterator_tag;

  sorted_inserter_iterator(Container &c) : m_container(&c) {}

  template <typename T> sorted_inserter_iterator &operator=(T &&object) {
    auto it = m_container->begin();
    auto end = m_container->end();

    while (it != end && *it < FWD(object)) {
      ++it;
    }

    m_container->insert(it, FWD(object));
    return *this;
  }

  sorted_inserter_iterator &operator++() { return *this; }
  sorted_inserter_iterator &operator*() { return *this; }

private:
  Container *m_container;
};

} // namespace detail

template <typename R> constexpr auto enumerate(R &&c) {
  typed_static_assert_msg(is_iterable(type_v<R>), "R must be an iterable");
  return detail::enumerate_range<decltype(std::begin(c))>{std::begin(c), std::end(c)};
}

template <typename Filter> constexpr auto filter(Filter filter) {
  return [f = std::move(filter)](auto &&c) {
    return detail::filter_range<decltype(std::begin(c)), Filter>{std::begin(c),
                                                                 std::end(c), f};
  };
}

template <typename MapFunction> constexpr auto map(MapFunction mapFunction) {
  return [f = std::move(mapFunction)](auto &&c) {
    return detail::map_range<decltype(std::begin(c)), MapFunction>{std::begin(c),
                                                                   std::end(c), f};
  };
}

template <typename Container> constexpr auto sorted_inserter(Container &c) {
  return ltl::detail::sorted_inserter_iterator<Container>{c};
}

template <typename... Containers> constexpr auto zip(Containers &&... cs) {
  return ltl::detail::zip_range<Containers...>{std::forward<Containers>(cs)...};
}

//////////////////// Algorithms
#define ALGO_MONO_ITERATOR(name)                                                         \
  template <typename C, typename... As, LTL_REQUIRE(is_iterable(type_v<C>))>             \
  auto name(C &&c, As &&... as) {                                                        \
    typed_static_assert(is_iterable(c));                                                 \
    return std::name(std::begin(c), std::end(c), FWD(as)...);                            \
  }

#define ALGO_DOUBLE_ITERATOR(name)                                                       \
  template <typename C1, typename C2, typename... As,                                    \
            LTL_REQUIRE(is_iterable(type_v<C1>) && is_iterable(type_v<C2>))>             \
  auto name(C1 &&c1, C2 &&c2, As &&... as) {                                             \
    return std::name(std::begin(c1), std::end(c1), std::begin(c2), std::end(c2),         \
                     FWD(as)...);                                                        \
  }

// Version for finds
#define ALGO_FIND_VALUE(name)                                                            \
  template <typename C, typename... As, LTL_REQUIRE(is_iterable(type_v<C>))>             \
  auto name(C &&c, As &&... as)->std::optional<decltype(c.begin())> {                    \
    auto it = std::name(std::begin(c), std::end(c), FWD(as)...);                         \
    if (it == std::end(c))                                                               \
      return std::nullopt;                                                               \
    return it;                                                                           \
  }                                                                                      \
  template <typename C, typename... As, LTL_REQUIRE(is_iterable(type_v<C>))>             \
  auto LPL_CAT(name, _by_value)(C && c, As && ... as)                                    \
      ->std::optional<std::decay_t<decltype(*FWD(c).begin())>> {                         \
    auto opt = name(FWD(c), FWD(as)...);                                                 \
    if (opt)                                                                             \
      return **opt;                                                                      \
    return std::nullopt;                                                                 \
  }

#define ALGO_FIND_RANGE(name)                                                            \
  template <typename C1, typename C2, typename... As,                                    \
            LTL_REQUIRE(is_iterable(type_v<C1>) && is_iterable(type_v<C2>))>             \
  auto name(C1 &&c1, C2 &&c2, As &&... as)->std::optional<decltype(c1.begin())> {        \
    auto it = std::name(std::begin(c1), std::end(c1), std::begin(c2), std::end(c2),      \
                        FWD(as)...);                                                     \
    if (it == std::end(c1))                                                              \
      return std::nullopt;                                                               \
    return it;                                                                           \
  }

// Version for finds
LPL_MAP(ALGO_FIND_VALUE, find, find_if, find_if_not, adjacent_find)
LPL_MAP(ALGO_FIND_RANGE, find_first_of, find_end, search)

// Non modifying
LPL_MAP(ALGO_MONO_ITERATOR, all_of, any_of, none_of, count, count_if, for_each, mismatch)

// Modifying
LPL_MAP(ALGO_MONO_ITERATOR, copy, copy_if, copy_backward, move, move_backward, fill,
        transform, generate)

LPL_MAP(ALGO_MONO_ITERATOR, remove, remove_if, remove_copy, remove_copy_if, replace,
        replace_if, replace_copy, replace_copy_if)

LPL_MAP(ALGO_MONO_ITERATOR, swap_ranges, reverse, reverse_copy, shuffle, sample, unique,
        unique_copy)

// Partitioning
LPL_MAP(ALGO_MONO_ITERATOR, is_partitioned, partition, partition_copy, stable_partition,
        partition_point)

// Sorting
LPL_MAP(ALGO_MONO_ITERATOR, is_sorted, is_sorted_until)

template <typename C, typename... P,
          LTL_REQUIRE(is_iterable(type_v<C>) && !is_const(remove_reference(type_v<C>)))>
C sort(C &&c, P &&... p) {
  std::sort(std::begin(c), std::end(c), FWD(p)...);
  return c;
}

template <typename C, typename... P,
          LTL_REQUIRE(is_iterable(type_v<C>) && !is_const(remove_reference(type_v<C>)))>
C stable_sort(C &&c, P &&... p) {
  std::stable_sort(std::begin(c), std::end(c), FWD(p)...);
  return c;
}

template <typename C,
          LTL_REQUIRE(is_iterable(type_v<C>) && is_const(remove_reference(type_v<C>)))>
[[nodiscard]] std::decay_t<C> sort(C &&c) {
  std::decay_t<C> newContainer;
  ltl::copy(c, ltl::sorted_inserter(newContainer));
  return newContainer;
}

// binary search operations
LPL_MAP(ALGO_MONO_ITERATOR, lower_bound, upper_bound, binary_search, equal_range)

// Other operations
ALGO_DOUBLE_ITERATOR(merge)
ALGO_MONO_ITERATOR(inplace_merge)

// set operations
LPL_MAP(ALGO_DOUBLE_ITERATOR, includes, set_difference, set_intersection,
        set_symmetric_difference, set_union)

// heap operation
LPL_MAP(ALGO_MONO_ITERATOR, is_heap, is_heap_until, make_heap, push_heap, pop_heap,
        sort_heap)

// max / min
LPL_MAP(ALGO_MONO_ITERATOR, max_element, min_element, minmax_element)

// comparison
LPL_MAP(ALGO_DOUBLE_ITERATOR, equal, lexicographical_compare)

// permutation
ALGO_DOUBLE_ITERATOR(is_permutation)
LPL_MAP(ALGO_MONO_ITERATOR, next_permutation, prev_permutation)

// numeric operations
LPL_MAP(ALGO_MONO_ITERATOR, iota, accumulate, inner_product, adjacent_difference,
        partial_sum)

#undef ALGO_MONO_ITERATOR
#undef ALGO_DOUBLE_ITERATOR
} // namespace ltl
