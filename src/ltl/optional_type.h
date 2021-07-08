/**
 * @file optional_type.h
 */
#pragma once

#include "traits.h"
#include "Range/AsPointer.h"

namespace ltl {
namespace detail {
struct EmptyOptionalType {};
} // namespace detail

template <typename T>
struct optional_type {
    constexpr optional_type() = default;
    constexpr optional_type(T) noexcept {}
    static constexpr true_t has_value{};

    constexpr auto operator*() const noexcept { return T{}; }
    constexpr auto operator->() const noexcept { return AsPointer{T{}}; }
};

template <>
struct optional_type<detail::EmptyOptionalType> {
    static constexpr false_t has_value{};
};

template <typename T>
optional_type(T) -> optional_type<T>;

constexpr optional_type<detail::EmptyOptionalType> nullopt_type;

LTL_MAKE_IS_KIND(optional_type, is_optional_type, is_optional_type_f, IsOptionalType, typename, );

} // namespace ltl
