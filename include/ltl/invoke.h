#pragma once

#include <type_traits>
#include <utility>

namespace ltl {

namespace details {

template <typename... Args>
struct type_from_function;

template <typename F, typename... Args>
struct type_from_function<std::void_t<decltype(std::declval<F>()(std::declval<Args>()...))>, F, Args...> {
    using type = decltype(std::declval<F>()(std::declval<Args>()...));
};

template <typename... Args>
struct type_from_method;

template <typename F, typename T, typename... Args>
struct type_from_method<std::void_t<decltype((std::declval<T>().*std::declval<F>())(std::declval<Args>()...))>, F, T,
                        Args...> {
    using type = decltype((std::declval<T>().*std::declval<F>())(std::declval<Args>()...));
};

template <typename... Args>
struct type_from_attribute;

template <typename F, typename T>
struct type_from_attribute<std::void_t<decltype(std::declval<T>().*std::declval<F>())>, F, T> {
    using type = decltype(std::declval<T>().*std::declval<F>());
};

template <typename F, typename... Args>
using sfinae_helper =
    typename std::conditional_t<std::is_member_function_pointer_v<std::decay_t<F>>,                     //
                                type_from_method<void, std::decay_t<F>, Args...>,                       //
                                std::conditional_t<std::is_member_object_pointer_v<std::decay_t<F>>,    //
                                                   type_from_attribute<void, std::decay_t<F>, Args...>, //
                                                   type_from_function<void, F, Args...>>>::type;

template <typename AlwaysVoid, typename... Args>
struct fast_invoke_result {};

template <typename F, typename... Args>
struct fast_invoke_result<std::void_t<sfinae_helper<F, Args...>>, F, Args...> {
    using type = sfinae_helper<F, Args...>;
};

// template <typename F, typename... Args>
// auto type_from_callable(F &&f, Args &&... args) -> decltype(static_cast<F &&>(f)(static_cast<Args &&>(args)...));
//
// template <typename Type, typename T, typename Class>
// auto type_from_callable(Type T::*f, Class &&x) -> decltype(static_cast<Class &&>(x).*f);
//
// template <typename Type, typename T, typename Class, typename... Args>
// auto type_from_callable(Type T::*f, Class &&x, Args &&... args)
//    -> decltype((static_cast<Class &&>(x).*f)(static_cast<Args &&>(args)...));

// template <typename AlwaysVoid, typename... Args>
// struct fast_invoke_result {};
//
// template <typename F, typename... Args>
// struct fast_invoke_result<std::void_t<decltype(type_from_callable(std::declval<F>(), std::declval<Args>()...))>, F,
//                          Args...> {
//    using type = decltype(type_from_callable(std::declval<F>(), std::declval<Args>()...));
//};

} // namespace details

template <typename F, typename... Args>
using fast_invoke_result = details::fast_invoke_result<void, F, Args...>;

template <typename F, typename... Args>
using fast_invoke_result_t = typename fast_invoke_result<F, Args...>::type;

namespace detail {

template <class T, class Type, class T1, class... Args>
constexpr decltype(auto) INVOKE(Type T::*f, T1 &&t1, Args &&... args) {
    if constexpr (std::is_member_function_pointer_v<decltype(f)>) {
        return (static_cast<T1 &&>(t1).*f)(static_cast<Args &&>(args)...);
    } else {
        return static_cast<T1 &&>(t1).*f;
    }
}

template <class F, class... Args>
constexpr decltype(auto) INVOKE(F &&f, Args &&... args) {
    return static_cast<F &&>(f)(static_cast<Args &&>(args)...);
}
} // namespace detail

template <class F, class... Args>
constexpr fast_invoke_result_t<F, Args...> invoke(F &&f,
                                                  Args &&... args) noexcept(std::is_nothrow_invocable_v<F, Args...>) {
    return detail::INVOKE(static_cast<F &&>(f), static_cast<Args &&>(args)...);
}
} // namespace ltl
