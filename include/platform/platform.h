#ifndef PLATFORM_H
#define PLATFORM_H

#include "game_types.h"
#include <xcb/xcb.h>

typedef struct DisplayInitInfo{
    u32 w,h;

    // pre determin the position of window
    // by default it is center
    // u32 x,y;
}DisplayInitInfo;

typedef struct DisplayState{
    u32 x,y;
    u32 width, hright;

    #ifdef WIN32
    //windows types
    #endif
    #ifdef __linux__
    xcb_connection_t *connection;
    xcb_window_t window;
    #endif
}DisplayState;

Result display_init(DisplayInitInfo info, DisplayState* out_state);
Result destroy_display(DisplayState* state);


#endif //PLATFORM_H