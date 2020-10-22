#pragma once

#include <type_traits>
#include <utility>

namespace ltl {

namespace details {

template <typename F, typename... Args>
struct type_from_function {
    using type = decltype(std::declval<F>()(std::declval<Args>()...));
};

template <typename F, typename T, typename... Args>
struct type_from_method {
    using type = decltype((std::declval<T>().*std::declval<F>())(std::declval<Args>()...));
};

template <typename F, typename T, typename... Args>
struct type_from_attribute {
    using type = decltype(std::declval<T>().*std::declval<F>());
};
} // namespace details

template <typename F, typename... Args>
using fast_invoke_result =
    std::conditional_t<std::is_member_function_pointer_v<std::remove_reference_t<F>>,                        //
                       details::type_from_method<std::remove_reference_t<F>, Args...>,                       //
                       std::conditional_t<std::is_member_object_pointer_v<std::remove_reference_t<F>>,       //
                                          details::type_from_attribute<std::remove_reference_t<F>, Args...>, //
                                          details::type_from_function<F, Args...>>>;

template <typename F, typename... Args>
using fast_invoke_result_t = typename fast_invoke_result<F, Args...>::type;

namespace detail {

template <class T, class Type, class T1, class... Args>
constexpr decltype(auto) FAST_INVOKE(Type T::*f, T1 &&t1, Args &&... args) {
    if constexpr (std::is_member_function_pointer_v<decltype(f)>) {
        return (static_cast<T1 &&>(t1).*f)(static_cast<Args &&>(args)...);
    } else {
        return static_cast<T1 &&>(t1).*f;
    }
}

template <class F, class... Args>
constexpr decltype(auto) FAST_INVOKE(F &&f, Args &&... args) {
    return static_cast<F &&>(f)(static_cast<Args &&>(args)...);
}

template <class>
constexpr bool is_reference_wrapper_v = false;
template <class U>
constexpr bool is_reference_wrapper_v<std::reference_wrapper<U>> = true;

template <class T, class Type, class T1, class... Args>
constexpr decltype(auto) INVOKE(Type T::*f, T1 &&t1, Args &&... args) {
    if constexpr (std::is_member_function_pointer_v<decltype(f)>) {
        if constexpr (std::is_base_of_v<T, std::decay_t<T1>>)
            return (std::forward<T1>(t1).*f)(std::forward<Args>(args)...);
        else if constexpr (is_reference_wrapper_v<std::decay_t<T1>>)
            return (t1.get().*f)(std::forward<Args>(args)...);
        else
            return ((*std::forward<T1>(t1)).*f)(std::forward<Args>(args)...);
    } else {
        static_assert(std::is_member_object_pointer_v<decltype(f)>);
        static_assert(sizeof...(args) == 0);
        if constexpr (std::is_base_of_v<T, std::decay_t<T1>>)
            return std::forward<T1>(t1).*f;
        else if constexpr (is_reference_wrapper_v<std::decay_t<T1>>)
            return t1.get().*f;
        else
            return (*std::forward<T1>(t1)).*f;
    }
}

template <class F, class... Args>
constexpr decltype(auto) INVOKE(F &&f, Args &&... args) {
    return std::forward<F>(f)(std::forward<Args>(args)...);
}

} // namespace detail

template <class F, class... Args>
constexpr fast_invoke_result_t<F, Args...>
fast_invoke(F &&f, Args &&... args) noexcept(std::is_nothrow_invocable_v<F, Args...>) {
    return detail::FAST_INVOKE(static_cast<F &&>(f), static_cast<Args &&>(args)...);
}

template <class F, class... Args>
constexpr std::invoke_result_t<F, Args...> invoke(F &&f,
                                                  Args &&... args) noexcept(std::is_nothrow_invocable_v<F, Args...>) {
    return detail::INVOKE(std::forward<F>(f), std::forward<Args>(args)...);
}

} // namespace ltl
