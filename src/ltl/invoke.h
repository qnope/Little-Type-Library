#pragma once

#include <type_traits>
#include <utility>

namespace ltl {

namespace detail {

template <class>
constexpr bool is_reference_wrapper_v = false;
template <class U>
constexpr bool is_reference_wrapper_v<std::reference_wrapper<U>> = true;

template <class T, class Type, class T1, class... Args>
constexpr decltype(auto) INVOKE(Type T::*f, T1 &&t1, Args &&... args) {
    if constexpr (std::is_member_function_pointer_v<decltype(f)>) {
        if constexpr (std::is_base_of_v<T, std::decay_t<T1>>)
            return (static_cast<T1 &&>(t1).*f)(static_cast<Args &&>(args)...);
        else if constexpr (is_reference_wrapper_v<std::decay_t<T1>>)
            return (t1.get().*f)(static_cast<Args &&>(args)...);
        else
            return ((*static_cast<T1 &&>(t1)).*f)(static_cast<Args &&>(args)...);
    } else {
        static_assert(std::is_member_object_pointer_v<decltype(f)>);
        static_assert(sizeof...(args) == 0);
        if constexpr (std::is_base_of_v<T, std::decay_t<T1>>)
            return static_cast<T1 &&>(t1).*f;
        else if constexpr (is_reference_wrapper_v<std::decay_t<T1>>)
            return t1.get().*f;
        else
            return (*static_cast<T1 &&>(t1)).*f;
    }
}

template <class F, class... Args>
constexpr decltype(auto) INVOKE(F &&f, Args &&... args) {
    return static_cast<F &&>(f)(static_cast<Args &&>(args)...);
}

} // namespace detail

template <class F, class... Args>
constexpr decltype(auto) fast_invoke(F &&f, Args &&... args) {
    return detail::INVOKE(static_cast<F &&>(f), static_cast<Args &&>(args)...);
}

template <class F, class... Args>
constexpr std::invoke_result_t<F, Args...> invoke(F &&f,
                                                  Args &&... args) noexcept(std::is_nothrow_invocable_v<F, Args...>) {
    return detail::INVOKE(static_cast<F &&>(f), static_cast<Args &&>(args)...);
}

} // namespace ltl
