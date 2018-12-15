#pragma once

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
