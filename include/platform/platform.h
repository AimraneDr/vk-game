#ifndef PLATFORM_H
#define PLATFORM_H

#include "game_types.h"
#include <X11/Xlib.h>

typedef struct DisplayInitInfo{
    u32 w,h;

    // pre determin the position of window
    // by default it is center
    // u32 x,y;
}DisplayInitInfo;

#ifdef __linux__
typedef struct DisplayProps{
    Atom wmDeleteWindow;
}DisplayProps;
#endif

typedef struct DisplayState{
    u32 x,y;
    u32 width, height;

    bool shouldClose;

    #ifdef WIN32
    //windows types
    #endif
    #ifdef __linux__
    Display* display;
    Window window;
    DisplayProps __internal;
    #endif
}DisplayState;

Result display_init(DisplayInitInfo info, DisplayState* out_state);
Result destroy_display(DisplayState* state);

Result display_PullEvents(DisplayState* state);


#endif //PLATFORM_H