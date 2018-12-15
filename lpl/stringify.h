#pragma once

#define LPL_STRINGIFY_IMPL(...) #__VA_ARGS__
#define LPL_STRINGIFY(...) LPL_STRINGIFY_IMPL(__VA_ARGS__)
