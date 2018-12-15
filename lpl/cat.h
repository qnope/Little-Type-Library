#pragma once

#define LPL_CAT_IMPL(x, ...) x##__VA_ARGS__
#define LPL_CAT(...) LPL_CAT_IMPL(__VA_ARGS__)
