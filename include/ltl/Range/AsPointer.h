#pragma once

#include <memory>
#include <utility>

namespace ltl {
template <typename T>
struct AsPointer {
    using underlying_type = T;
    AsPointer(T &&v) noexcept : v{std::move(v)} {}
    T &operator*() noexcept { return v; }
    T *operator->() noexcept { return std::addressof(v); }
    T v;
};

template <typename T>
struct AsPointer<T &> {
    using underlying_type = T &;
    AsPointer(T &v) noexcept : v{std::addressof(v)} {}
    T &operator*() noexcept { return *v; }
    T *operator->() noexcept { return v; }
    T *v;
};
} // namespace ltl
