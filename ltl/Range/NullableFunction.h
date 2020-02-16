#pragma once

#include <memory>

#include "../invoke.h"

namespace ltl {
template <typename F> struct NullableFunction {
  struct DestructorPtr {
    void operator()(F *x) { x->~F(); }
  };

  NullableFunction() = default;
  NullableFunction(F f) : m_ptr{new (&m_memory) F{std::move(f)}} {}
  NullableFunction(const NullableFunction &f) {
    if (f.m_ptr)
      m_ptr.reset(new (&m_memory) F{std::move(*f.m_ptr)});
  }

  NullableFunction &operator=(NullableFunction f) {
    m_ptr = nullptr;
    if (f.m_ptr)
      m_ptr.reset(new (&m_memory) F{std::move(*f.m_ptr)});
    return *this;
  }

  template <typename... Args> decltype(auto) operator()(Args &&... args) const {
    assert(m_ptr);
    return ltl::invoke(*m_ptr, FWD(args)...);
  }

  std::aligned_storage_t<sizeof(F), alignof(F)> m_memory;
  std::unique_ptr<F, DestructorPtr> m_ptr;
};

struct Nothing {};

} // namespace ltl
