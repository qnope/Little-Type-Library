#pragma once

#include "Tuple.h"

namespace ltl {
//////////////////////////////// Y_Combinator /////////////////////////////////
template <typename F> struct fix : F {
  template <typename... Args>
  constexpr decltype(auto) operator()(Args &&... args) {
    return static_cast<F &>(*this)(*this, FWD(args)...);
  }
};
template <typename F> fix(F)->fix<F>;

template <typename F, typename... Args>
constexpr auto report_call(F f, Args... xs) {
  return [f = std::move(f), xs = tuple_t{std::move(xs)...}](auto &&... _ys) {
    auto ys = tuple_t<decltype(_ys)...>(FWD(_ys)...);
    auto args = xs + ys;
    return args([f](auto &&... args) { return f(FWD(args)...); });
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
