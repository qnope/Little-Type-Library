#pragma once

#include "Tuple.h"

namespace ltl {
//////////////////////////////// Y_Combinator /////////////////////////////////
template <typename F> struct fix : F {
  template <typename... Args>
  constexpr decltype(auto) operator()(Args &&... args) const {
    return static_cast<const F &>(*this)(*this, FWD(args)...);
  }

  template <typename... Args>
  constexpr decltype(auto) operator()(Args &&... args) {
    return static_cast<F &>(*this)(*this, FWD(args)...);
  }
};
template <typename F> fix(F)->fix<F>;

template <typename F, typename... Args>
constexpr auto report_call(F f, Args... xs) {
  return [f = std::move(f),
          xs = tuple_t{std::move(xs)...}](auto &&... _ys) -> decltype(auto) {
    auto ys = tuple_t<decltype(_ys)...>(FWD(_ys)...);
    auto args = xs + std::move(ys);
    return args(
        [f](auto &&... args) -> decltype(auto) { return f(FWD(args)...); });
  };
}

template <typename F, typename... Args>
constexpr auto curry(F f, Args &&... args) {
  if constexpr (ltl::is_invocable(type_from(f), type_from(args)...)) {
    return f(FWD(args)...);
  } else {
    return report_call(lift(curry), std::move(f), FWD(args)...);
  }
}

} // namespace ltl
