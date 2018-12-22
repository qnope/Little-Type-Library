#pragma once
#include "type_t.h"
#include <type_traits>
#include <utility>

namespace ltl {
template <typename... Fs> struct overloader : Fs... {
  constexpr overloader(Fs &&... fs) : Fs{std::move(fs)}... {}

  using Fs::operator()...;
};

template <typename... Fs> overloader(Fs...)->overloader<Fs...>;

} // namespace ltl
