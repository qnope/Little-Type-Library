/**
 * @file NullableFunction.h
 */
#pragma once

#include <cassert>
#include <cstddef>
#include <optional>
#include <type_traits>
#include "ltl/ltl.h"
#include "ltl/invoke.h"

namespace ltl {

template <typename F>
struct NullableFunction {
    constexpr NullableFunction() = default;
    constexpr NullableFunction(F f) : m_function{std::move(f)} {}

    constexpr NullableFunction(const NullableFunction &) = default;
    constexpr NullableFunction(NullableFunction &&) = default;

    constexpr NullableFunction &operator=(NullableFunction f) {
        m_function.reset();
        if (f.m_function)
            m_function.emplace(std::move(*f.m_function));
        return *this;
    }

    template <typename... Args>
    constexpr decltype(auto) operator()(Args &&...args) const {
        return ltl::fast_invoke(*m_function, FWD(args)...);
    }

    mutable std::optional<F> m_function;
};

} // namespace ltl
