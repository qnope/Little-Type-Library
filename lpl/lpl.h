#pragma once

#define LPL_STRINGIFY_IMPL(...) #__VA_ARGS__
#define LPL_STRINGIFY(...) LPL_STRINGIFY_IMPL(__VA_ARGS__)

#define LPL_CAT_IMPL(x, ...) x##__VA_ARGS__
#define LPL_CAT(...) LPL_CAT_IMPL(__VA_ARGS__)

#define LPL_DEC_1 0
#define LPL_DEC_2 1
#define LPL_DEC_3 2
#define LPL_DEC_4 3
#define LPL_DEC_5 4
#define LPL_DEC_6 5
#define LPL_DEC_7 6
#define LPL_DEC_8 7
#define LPL_DEC_9 8
#define LPL_DEC(...) LPL_CAT(LPL_DEC_, LPL_HEAD(__VA_ARGS__, ()))

#define LPL_INC_0 1
#define LPL_INC_1 2
#define LPL_INC_2 3
#define LPL_INC_3 4
#define LPL_INC_4 5
#define LPL_INC_5 6
#define LPL_INC_6 7
#define LPL_INC_7 8
#define LPL_INC_8 9
#define LPL_INC(...) LPL_CAT(LPL_INC_, LPL_HEAD(__VA_ARGS__, ()))

#define LPL_COMPL_1 0
#define LPL_COMPL_0 1
#define LPL_COMPL(...) LPL_CAT(LPL_COMPL_, __VA_ARGS__)

#define LPL_IDENTITY(x) x

#define EMPTY()
#define LPL_EXPAND(...) __VA_ARGS__
#define LPL_EAT(...)
#define LPL_DEFER(...) __VA_ARGS__ EMPTY()
#define LPL_DEFER_TWICE(...) __VA_ARGS__ EMPTY EMPTY()()
#define LPL_DEFER_THRICE(...) __VA_ARGS__ EMPTY EMPTY EMPTY()()()

#define LPL_EVAL1(...) LPL_EXPAND(LPL_EXPAND(__VA_ARGS__))
#define LPL_EVAL2(...) LPL_EVAL1(LPL_EVAL1(__VA_ARGS__))
#define LPL_EVAL3(...) LPL_EVAL2(LPL_EVAL2(__VA_ARGS__))
#define LPL_EVAL4(...) LPL_EVAL3(LPL_EVAL3(__VA_ARGS__))
#define LPL_EVAL5(...) LPL_EVAL4(LPL_EVAL4(__VA_ARGS__))
#define LPL_EVAL6(...) LPL_EVAL5(LPL_EVAL5(__VA_ARGS__))
#define LPL_EVAL(...) LPL_EVAL6(LPL_EVAL6(__VA_ARGS__))

#define LPL_HEAD(x, ...) x
#define LPL_SECOND(x, y, ...) y
#define LPL_TAIL(x, ...) __VA_ARGS__

#define LPL_STRIP_PARENTHESES_IMPL(...) __VA_ARGS__
#define LPL_STRIP_PARENTHESES(x) LPL_STRIP_PARENTHESES_IMPL x

#define LPL_PROBE UNUSED, 1
#define LPL_CHECK_IMPL(unused, n, ...) n
#define LPL_CHECK(expressionToTest) LPL_CHECK_IMPL(expressionToTest, 0, 0)

#define LPL_IS_PARENTHESES_IMPL(...) LPL_PROBE
#define LPL_IS_PARENTHESES(x) LPL_CHECK(LPL_IS_PARENTHESES_IMPL x)

#define LPL_IS_NOT_PARENTHESES(x) LPL_COMPL(LPL_IS_PARENTHESES(x))

#define LPL_IS_0_0 LPL_PROBE
#define LPL_IS_0(...) LPL_CHECK(LPL_CAT(LPL_IS_0_, LPL_HEAD(__VA_ARGS__, ())))

#define LPL_IS_NOT_0(...) LPL_COMPL(LPL_IS_0(LPL_HEAD(__VA_ARGS__, ())))

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
