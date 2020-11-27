#pragma once

#include "ltl.h"
#include <array>

#define qualifier_from(x) ltl::qualifier_v<ltl::getQualifierEnum(type_from(x))>

namespace ltl {
// Uniformize declval
template <typename T>
std::add_rvalue_reference_t<T> declval(type_t<T>);
template <typename T>
std::add_rvalue_reference_t<T> declval(T &&);

#define TRAIT(name)                                                                                                    \
    [[maybe_unused]] constexpr auto name = [](auto &&... xs) constexpr noexcept {                                      \
        return bool_v<std::LPL_CAT(name, _v) < std::decay_t<decltype(declval(FWD(xs)))>...> > ;                        \
    };

// Primary type categories
TRAIT(is_void)
TRAIT(is_null_pointer)
TRAIT(is_integral)
TRAIT(is_floating_point)
TRAIT(is_enum)
TRAIT(is_union)
TRAIT(is_class)
TRAIT(is_function)
TRAIT(is_pointer)
TRAIT(is_member_object_pointer)
TRAIT(is_member_function_pointer)

// Composite type categories
TRAIT(is_fundamental)
TRAIT(is_arithmetic)
TRAIT(is_scalar)
TRAIT(is_object)
TRAIT(is_compound)
TRAIT(is_member_pointer)

// Type properties
TRAIT(is_trivial)
TRAIT(is_trivially_copyable)
TRAIT(is_standard_layout)
TRAIT(is_empty)
TRAIT(is_polymorphic)
TRAIT(is_abstract)
TRAIT(is_final)
TRAIT(is_aggregate)
TRAIT(is_signed)
TRAIT(is_unsigned)

// Supported operations
TRAIT(is_default_constructible)
TRAIT(is_trivially_default_constructible)
TRAIT(is_nothrow_default_constructible)
TRAIT(is_copy_constructible)
TRAIT(is_trivially_copy_constructible)
TRAIT(is_nothrow_copy_constructible)
TRAIT(is_move_constructible)
TRAIT(is_trivially_move_constructible)
TRAIT(is_nothrow_move_constructible)

TRAIT(is_copy_assignable)
TRAIT(is_trivially_copy_assignable)
TRAIT(is_nothrow_copy_assignable)
TRAIT(is_move_assignable)
TRAIT(is_trivially_move_assignable)
TRAIT(is_nothrow_move_assignable)

TRAIT(is_assignable)
TRAIT(is_trivially_assignable)
TRAIT(is_nothrow_assignable)
TRAIT(is_swappable_with)
TRAIT(is_nothrow_swappable_with)

TRAIT(is_trivially_destructible)
TRAIT(is_nothrow_destructible)
TRAIT(has_virtual_destructor)
TRAIT(is_swappable)
TRAIT(is_nothrow_swappable)

TRAIT(is_trivially_constructible)
TRAIT(is_nothrow_constructible)

TRAIT(is_base_of)
TRAIT(is_convertible)

#undef TRAIT

#define TRAIT_REFERENCE(name)                                                                                          \
    [[maybe_unused]] constexpr auto name = [](auto &&... xs) constexpr noexcept {                                      \
        return bool_v<std::LPL_CAT(name, _v) < decltype(declval(FWD(xs)))...> > ;                                      \
    };

#define TRAIT_CVNESS(name)                                                                                             \
    [[maybe_unused]] constexpr auto name = [](auto &&x) constexpr {                                                    \
        return bool_v<std::LPL_CAT(name, _v) < std::remove_reference_t<decltype(declval(FWD(x)))>> > ;                 \
    };

// Reference / cv-ness
TRAIT_REFERENCE(is_lvalue_reference)
TRAIT_REFERENCE(is_rvalue_reference)
TRAIT_CVNESS(is_const)
TRAIT_CVNESS(is_volatile)
TRAIT_CVNESS(is_array)

#define TRAIT(name)                                                                                                    \
    template <typename T>                                                                                              \
        [[nodiscard]] constexpr number_t<std::LPL_CAT(name, _v) < T> > name(type_t<T>) {                               \
        return {};                                                                                                     \
    }

// Property queries
template <typename T, int N = 0>
[[nodiscard]] constexpr number_t<std::extent_v<T, N>> extent(type_t<T>, number_t<N> = number_t<N>{}) {
    return {};
}

TRAIT(alignment_of)
TRAIT(rank)

#undef TRAIT

#define TRAIT(name)                                                                                                    \
    [[maybe_unused]] constexpr auto name = [](auto x) constexpr noexcept {                                             \
        return type_v<std::LPL_CAT(name, _t) < decltype_t(x)> > ;                                                      \
    };

// const-volatibility specifiers
TRAIT(remove_cv)
TRAIT(remove_const)
TRAIT(remove_volatile)
TRAIT(add_cv)
TRAIT(add_const)
TRAIT(add_volatile)

// references
TRAIT(remove_reference)
TRAIT(add_lvalue_reference)
TRAIT(add_rvalue_reference)

// pointers
TRAIT(remove_pointer)
TRAIT(add_pointer)

// Sign modifiers
TRAIT(make_signed)
TRAIT(make_unsigned)

// Arrays
TRAIT(remove_extent)
TRAIT(remove_all_extents)
TRAIT(decay)
#undef TRAIT

/////////////////////////// type qualifier //////////////////////////////////
enum class qualifier_enum {
    NO_CV = 1 << 0,
    VOLATILE = 1 << 1,
    CONST_ = 1 << 2,
    NO_REF = 1 << 3,
    LVALUE_REF = 1 << 4,
    RVALUE_REF = 1 << 5
};

constexpr qualifier_enum &operator|=(qualifier_enum &a, qualifier_enum b) noexcept {
    a = static_cast<qualifier_enum>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
    return a;
}

constexpr qualifier_enum operator|(qualifier_enum a, qualifier_enum b) noexcept {
    a |= b;
    return a;
}

constexpr qualifier_enum &operator^=(qualifier_enum &a, qualifier_enum b) noexcept {
    a = static_cast<qualifier_enum>(static_cast<unsigned int>(a) ^ static_cast<unsigned int>(b));
    return a;
}

constexpr qualifier_enum operator^(qualifier_enum a, qualifier_enum b) noexcept {
    a ^= b;
    return a;
}

constexpr bool operator&(qualifier_enum a, qualifier_enum b) noexcept {
    return (static_cast<unsigned int>(a) & static_cast<unsigned int>(b)) == static_cast<unsigned int>(b);
}

template <qualifier_enum e>
struct qualifier_t {};
template <qualifier_enum e>
constexpr qualifier_t<e> qualifier_v{};

template <typename T>
[[nodiscard]] constexpr qualifier_enum getRefQualifierEnum(type_t<T>) noexcept {
    qualifier_enum result{};
    if constexpr (std::is_lvalue_reference_v<T>) {
        result |= qualifier_enum::LVALUE_REF;
    }

    if constexpr (std::is_rvalue_reference_v<T>) {
        result |= qualifier_enum::RVALUE_REF;
    }
    return result;
}

template <typename T>
[[nodiscard]] constexpr qualifier_enum getCVQualifierEnum(type_t<T>) noexcept {
    qualifier_enum result{};
    if constexpr (std::is_const_v<std::remove_reference_t<T>>) {
        result |= qualifier_enum::CONST_;
    }

    if constexpr (std::is_volatile_v<std::remove_reference_t<T>>) {
        result |= qualifier_enum::VOLATILE;
    }
    return result;
}

template <typename T>
[[nodiscard]] constexpr qualifier_enum getQualifierEnum(type_t<T> t) noexcept {
    return getCVQualifierEnum(t) | getRefQualifierEnum(t);
}

template <qualifier_enum a, qualifier_enum b>
[[nodiscard]] constexpr qualifier_t<a | b> operator+(qualifier_t<a>, qualifier_t<b>) noexcept {
    return {};
}

template <typename T, qualifier_enum a>
[[nodiscard]] constexpr auto operator+(type_t<T> t, qualifier_t<a>) noexcept {
    if constexpr (a & qualifier_enum::CONST_)
        return add_const(remove_reference(t)) + qualifier_v<(a ^ qualifier_enum::CONST_) | getRefQualifierEnum(t)>;

    else if constexpr (a & qualifier_enum::VOLATILE)
        return add_volatile(remove_reference(t)) + qualifier_v<a ^ qualifier_enum::VOLATILE | getRefQualifierEnum(t)>;

    else if constexpr (a & qualifier_enum::LVALUE_REF)
        return add_lvalue_reference(t) + qualifier_v<a ^ qualifier_enum::LVALUE_REF>;

    else if constexpr (a & qualifier_enum::RVALUE_REF)
        return add_rvalue_reference(t) + qualifier_v<a ^ qualifier_enum::RVALUE_REF>;
    else
        return t;
}

template <qualifier_enum a, typename T>
[[nodiscard]] constexpr auto operator+(qualifier_t<a> q, type_t<T> t) noexcept {
    return t + q;
}

///////////////////////// is_valid

namespace detail {

template <typename F, typename = void, typename... Args>
constexpr bool is_validImpl = false;

template <typename F, typename... Args>
constexpr bool
    is_validImpl<F, std::void_t<decltype(ltl::invoke(std::declval<F>(), declval(std::declval<Args>())...))>, Args...> =
        true;
} // namespace detail

template <typename F>
constexpr auto is_valid(F &&) {
    return [](auto &&... xs) { return bool_t<detail::is_validImpl<F, void, decltype(xs)...>>{}; };
}

#define LTL_WRITE_AUTO_WITH_COMMA_IMPL(x) , auto &&x

#define LTL_WRITE_AUTO_IMPL(x, ...) (auto &&x LPL_MAP(LTL_WRITE_AUTO_WITH_COMMA_IMPL, __VA_ARGS__))

#define LTL_ENSURE_NOT_EMPTY(...) (__VA_ARGS__, ())
#define IS_VALID(variables, ...)                                                                                       \
    ltl::is_valid(                                                                                                     \
        [] LPL_IDENTITY(LTL_WRITE_AUTO_IMPL LTL_ENSURE_NOT_EMPTY variables) -> decltype(__VA_ARGS__, void()) {})

#define LTL_MAKE_IS_KIND(type, nameStruct, nameLambda, conceptName, templateType, v)                                   \
    template <typename T>                                                                                              \
    struct nameStruct {                                                                                                \
        static constexpr auto value = false;                                                                           \
    };                                                                                                                 \
    template <templateType v Ts>                                                                                       \
    struct nameStruct<type<Ts v>> {                                                                                    \
        static constexpr auto value = true;                                                                            \
    };                                                                                                                 \
    template <typename T>                                                                                              \
    [[maybe_unused]] constexpr bool conceptName = nameStruct<std::decay_t<T>>::value;                                  \
    [[maybe_unused]] constexpr auto nameLambda = [](auto &&x) constexpr noexcept {                                     \
        return bool_t<conceptName<decltype(::ltl::declval(x))>>{};                                                     \
    }

template <typename T>
constexpr auto is_type(type_t<T> type) {
    return [type](auto &&x) { return decay_from(declval(FWD(x))) == type; };
}

template <typename T>
constexpr auto is_derived_from(type_t<T> type) {
    return [type](auto &&x) { return is_base_of(type, decay_from(declval(FWD(x)))); };
}

constexpr auto is_invocable = [](auto &&f, auto &&... args) {
    return bool_v<std::is_invocable_v<decltype(f), decltype(args)...>>;
};

template <typename F, typename... Ts>
constexpr auto invoke_result(type_t<F>, type_t<Ts>...) noexcept {
    return type_v<::std::invoke_result_t<F, Ts...>>;
}

template <typename ResultType, typename T>
constexpr auto copy_qualifier(type_t<T> type) {
    return type_v<ResultType> + qualifier_v<getQualifierEnum(type)>;
}

template <qualifier_enum q>
constexpr auto add_qualifier(qualifier_t<q>) {
    return [](auto x) constexpr { return x + qualifier_v<q>; };
}

using std::begin;
using std::end;

struct any_trait_t {
    constexpr any_trait_t() noexcept = default;
    template <typename T>
    constexpr any_trait_t(T &&) noexcept {}
};

constexpr any_trait_t any_trait_v;

constexpr auto is_iterable = IS_VALID((x), begin(x), end(x));
constexpr auto is_generic_callable = IS_VALID((x), x(any_trait_v));

constexpr ltl::false_t is_fixed_size_array(...);

template <typename T, std::size_t N>
constexpr ltl::true_t is_fixed_size_array(const T (&)[N]);

template <typename T, std::size_t N>
constexpr ltl::true_t is_fixed_size_array(const std::array<T, N> &);

} // namespace ltl
