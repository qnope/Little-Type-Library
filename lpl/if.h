#pragma once

#define LPL_IF_0(t, f) f
#define LPL_IF_1(t, f) t
#define LPL_IF(c) LPL_CAT(LPL_IF_, LPL_IS_NOT_0(c))

#define LPL_WHEN(c) LPL_IF(c)(LPL_EXPAND, LPL_EAT)

// WHILE
#define LPL_WHILE_IMPL(x, predicat, macroToApplyToX, motif)                    \
  LPL_WHEN(predicat(x))                                                        \
  (motif(x) LPL_DEFER_TWICE(LPL_WHILE_IMPL_I)()(macroToApplyToX(x), predicat,  \
                                                macroToApplyToX, motif))

#define LPL_WHILE_IMPL_I() LPL_WHILE_IMPL

#define LPL_WHILE(x, predicat, macroToApplyToX, motif)                         \
  LPL_EVAL(LPL_WHILE_IMPL(x, predicat, macroToApplyToX, motif))

// MAP
#define LPL_MAP_IMPL_PARENTHESES_1(...)
#define LPL_MAP_IMPL_PARENTHESES_0(macroToApply, x, ...)                       \
  macroToApply(x) LPL_DEFER(LPL_MAP_IMPL_I)()(macroToApply, __VA_ARGS__)

#define LPL_MAP_IMPL(macroToApply, x, ...)                                     \
  LPL_CAT(LPL_MAP_IMPL_PARENTHESES_, LPL_IS_PARENTHESES(x))                    \
  (macroToApply, x, __VA_ARGS__)

#define LPL_MAP_IMPL_I() LPL_MAP_IMPL

#define LPL_MAP(macroToApply, ...)                                             \
  LPL_EVAL(LPL_MAP_IMPL(macroToApply, __VA_ARGS__, (), ()))
