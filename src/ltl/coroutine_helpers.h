#pragma once

#include "ltl.h"

#if LTL_CPP20

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
