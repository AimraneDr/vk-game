#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include "defines.h"

#define RESULT_CODE_SUCCESS 0
#define RESULT_CODE_FAILED_TO_MAKE_XCB_CONX 1
#define RESULT_CODE_UNDEFINED_ERROR (u16)-1

typedef u16 Result;

typedef struct Display {
    char* title;
    u32 x,y;
    u32 w,h;
}Display;

#endif //GAME_TYPES_H