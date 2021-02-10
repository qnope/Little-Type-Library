#pragma once

#include "../Tuple.h"
#include "Filter.h"
#include "Map.h"

namespace ltl {
inline auto dereference() noexcept {
    return [](auto &&x) noexcept -> remove_rvalue_reference_t<decltype(*FWD(x))> { return *FWD(x); };
}

inline auto remove_null() noexcept {
    return filter([](auto &&x) noexcept { return static_cast<bool>(FWD(x)); });
}

inline auto to_ptr = [](auto &x) noexcept { return std::addressof(x); };

inline auto to_pair = [](auto &&tuple) {
    using Tuple = std::decay_t<decltype(tuple)>;
    static_assert(std::tuple_size_v<Tuple> == 2, "Tuple must have two elements");
    using pair = std::pair<std::tuple_element_t<0, Tuple>, std::tuple_element_t<1, Tuple>>;
    return pair{FWD(tuple)[0_n], FWD(tuple)[1_n]};
};

template <int... Ns>
auto get(number_t<Ns>...) {
    return map([](auto &&x) noexcept -> decltype(auto) {
        static_assert(sizeof...(Ns) > 0, "You must provide at least one value to get");

        if constexpr (sizeof...(Ns) == 1) {
            return (::std::get<static_cast<std::size_t>(Ns)>(FWD(x)), ...);
        }

        else {
            return ltl::tuple_t<decltype(::std::get<static_cast<std::size_t>(Ns)>(FWD(x)))...>{
                ::std::get<static_cast<std::size_t>(Ns)>(FWD(x))...};
        }
    });
}

inline auto keys() { return get(0_n); }
inline auto values() { return get(1_n); }

} // namespace ltl
