#pragma once
#include "type_t.h"
#include <type_traits>
#include <utility>

namespace ltl {
template <typename... Fs> struct overloader : Fs... {
  template <typename... _Fs>
  constexpr overloader(_Fs &&... fs) : Fs{std::forward<_Fs>(fs)}... {}

  using Fs::operator()...;
};

template <typename... Fs> overloader(Fs &&...)->overloader<std::decay_t<Fs>...>;

} // namespace ltl
