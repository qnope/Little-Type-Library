#pragma once

#include "traits.h"
#include "Range/AsPointer.h"

namespace ltl {

template <typename... Ts>
struct optional_type;

template <typename T>
struct optional_type<T> {
  constexpr optional_type() = default;
  constexpr optional_type(T) noexcept {}
  static constexpr true_t has_value{};

  constexpr auto operator*() const noexcept { return T{}; }
  constexpr auto operator-> () const noexcept { return AsPointer{T{}}; }
};

template <typename T>
optional_type(T)->optional_type<T>;

template <>
struct optional_type<> {
  static constexpr false_t has_value{};
};

constexpr optional_type<> nullopt_type;

LTL_MAKE_IS_KIND(optional_type, is_optional_type, IsOptionalType, typename);

} // namespace ltl
