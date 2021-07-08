/**
 * @file TypedTuple.h
 */
#pragma once

#include "Tuple.h"
#include "tuple_algos.h"

namespace ltl {
template <typename... Ts>
/**
 * @brief The TypedTuple struct
 */
struct TypedTuple : tuple_t<Ts...> {
    using type_list = fast::type_list<Ts...>;
    static_assert(fast::is_unique<type_list>::value, "Types must appear only once");

    using tuple_t<Ts...>::length;
    using tuple_t<Ts...>::isEmpty;

    TypedTuple() = default;
    TypedTuple(Ts... xs) : tuple_t<Ts...>{std::move(xs)...} {}

    template <typename T>
        [[nodiscard]] constexpr auto &get() & noexcept {
        constexpr auto index = fast::find<T, type_list>::value;
        static_assert(index, "Typed Tuple must have the type given to get");
        return static_cast<tuple_t<Ts...> &>(*this).template get<*index>();
    }

    template <typename T>
    [[nodiscard]] constexpr const auto &get() const &noexcept {
        constexpr auto index = fast::find<T, type_list>::value;
        static_assert(index, "Typed Tuple must have the type given to get");
        return static_cast<const tuple_t<Ts...> &>(*this).template get<*index>();
    }

    template <typename T>
        [[nodiscard]] constexpr auto get() && noexcept {
        constexpr auto index = fast::find<T, type_list>::value;
        static_assert(index, "Typed Tuple must have the type given to get");
        return static_cast<tuple_t<Ts...> &&>(std::move(*this)).template get<*index>();
    }
};

template <typename... Ts>
TypedTuple(Ts &&...)->TypedTuple<decay_reference_wrapper_t<Ts>...>;

template <typename... Ts>
struct is_tuple<TypedTuple<Ts...>> {
    static constexpr bool value = true;
};

} // namespace ltl
