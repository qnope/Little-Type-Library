#pragma once

#include "traits.h"
#include "Range/AsPointer.h"

namespace ltl {

namespace details {
struct EmptyOptionalType {};
} // namespace details

template <typename T>
struct optional_type {
    constexpr optional_type() = default;
    constexpr optional_type(T) noexcept {}
    static constexpr true_t has_value{};

    constexpr auto operator*() const noexcept { return T{}; }
    constexpr auto operator-> () const noexcept { return AsPointer{T{}}; }
};

template <>
struct optional_type<details::EmptyOptionalType> {
    static constexpr false_t has_value{};
};

template <typename T>
optional_type(T)->optional_type<T>;

constexpr optional_type<details::EmptyOptionalType> nullopt_type;

LTL_MAKE_IS_KIND(optional_type, is_optional_type, is_optional_type_f, IsOptionalType, typename, );

} // namespace ltl
