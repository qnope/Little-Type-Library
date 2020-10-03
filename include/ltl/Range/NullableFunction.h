#pragma once

#include <optional>
#include "../ltl.h"
#include "../invoke.h"

namespace ltl {
template <typename F>
struct NullableFunction {
    NullableFunction() = default;
    NullableFunction(F f) : m_function{std::move(f)} {}
    NullableFunction(const NullableFunction &f) {
        if (f.m_function)
            m_function.emplace(*f.m_function);
    }

    NullableFunction &operator=(NullableFunction f) {
        if (f.m_function)
            m_function.emplace(std::move(*f.m_function));
        return *this;
    }

    template <typename... Args>
    decltype(auto) operator()(Args &&... args) const {
        assert(m_function);
        return ltl::invoke(*m_function, FWD(args)...);
    }

    mutable std::optional<F> m_function;
};

struct Nothing {};

} // namespace ltl
