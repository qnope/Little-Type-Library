#pragma once

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
