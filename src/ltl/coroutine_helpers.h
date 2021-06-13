/**
 * @file coroutine_helpers.h
 */
#pragma once

#include "ltl.h"

#if defined(__cpp_impl_coroutine) || defined(__cpp_lib_coroutine)
#define LTL_COROUTINE 1
#else
#define LTL_COROUTINE 0
#endif

#if LTL_COROUTINE
#include <coroutine>

namespace ltl {
template <typename T>
struct promise_type {
    T *resultObject;
    T get_return_object() noexcept { return {*this}; }
    std::suspend_never initial_suspend() noexcept { return {}; }
    std::suspend_never final_suspend() noexcept { return {}; }

    void return_value(T result) noexcept { *resultObject = std::move(result); }
    void unhandled_exception() noexcept {}
};
} // namespace ltl
#endif
