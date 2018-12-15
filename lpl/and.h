#pragma once
#include "evaluation.h"

#define LPL_AND_IMPL(x, ...) LPL_WHEN(LPL_IS_0(x))(, 0)

/* LPL_SECOND(UNUSED, 0, 1, UNUSED) if 0
   LPL_SECOND(UNUSED, 1, UNUSED) if 1 */
#define LPL_AND(...)                                                           \
  LPL_EVAL(LPL_DEFER(LPL_SECOND)(UNUSED LPL_MAP(LPL_AND_IMPL, __VA_ARGS__), 1, \
                                 UNUSED))

#define LPL_OR_IMPL(x, ...) LPL_WHEN(LPL_IS_NOT_0(x))(, 1)

#define LPL_OR(...)                                                            \
  LPL_EVAL(LPL_DEFER(LPL_SECOND)(UNUSED LPL_MAP(LPL_OR_IMPL, __VA_ARGS__), 0,  \
                                 UNUSED))
