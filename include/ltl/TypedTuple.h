#pragma once

#include "Tuple.h"
#include "tuple_algos.h"

namespace ltl {
template <typename... Ts>
class TypedTuple : public tuple_t<Ts...> {
    static constexpr auto types = type_list_v<Ts...>;
    typed_static_assert_msg(is_unique_type(types), "Types must appear only once");

  public:
    using tuple_t<Ts...>::tuple_t;

    using tuple_t<Ts...>::length;
    using tuple_t<Ts...>::isEmpty;

    using tuple_t<Ts...>::get;

    template <typename T>
    [[nodiscard]] constexpr auto &get() noexcept {
        constexpr auto index = find_type(types, type_v<T>);
        static_assert(index.has_value, "Type must be in the type list");
        return (*this)[*index];
    }

    template <typename T>
    [[nodiscard]] constexpr const auto &get() const noexcept {
        constexpr auto index = find_type(types, type_v<T>);
        static_assert(index.has_value, "Type must be in the type list");
        return (*this)[*index];
    }
};

template <typename... Ts>
struct is_tuple<TypedTuple<Ts...>> {
    static constexpr bool value = true;
};

} // namespace ltl
