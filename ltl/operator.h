#pragma once

#include "concept.h"

#define _LAMBDA_WRITE_AUTO(x, y, ...)                                          \
  auto &&x LPL_WHEN(LPL_IS_NOT_PARENTHESES(y))(                                \
      LPL_CAT(, , ) LPL_DEFER_TWICE(_LAMBDA_WRITE_AUTO_I)()(y, __VA_ARGS__))

#define _LAMBDA_WRITE_AUTO_I() _LAMBDA_WRITE_AUTO
#define _(variables, expr)                                                     \
  [](LPL_EVAL(LPL_DEFER(_LAMBDA_WRITE_AUTO)(LPL_STRIP_PARENTHESES(variables),  \
                                            (), ()))) { return expr; }

template <typename Opt, typename F>
constexpr auto operator|(Opt &&opt, F &&f)
    -> ltl::requires_t<std::optional<std::decay_t<decltype(FWD(f)(*FWD(opt)))>>,
                       ::ltl::IsOptional<Opt>> {
  if (FWD(opt))
    return FWD(f)(*FWD(opt));
  return std::nullopt;
}

template <typename Opt, typename F>
constexpr auto operator>>(Opt &&opt, F &&f)
    -> ltl::requires_t<decltype(FWD(f)(*FWD(opt))), ::ltl::IsOptional<Opt>> {
  typed_static_assert_msg(
      ::ltl::is_optional(FWD(f)(*FWD(opt))),
      "Binding requires the function to return an optional");
  if (FWD(opt))
    return FWD(f)(*FWD(opt));
  return std::nullopt;
}

#define LIFT(f) [](auto &&... xs) { return f(FWD(xs)...); }
