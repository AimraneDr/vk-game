#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

#ifdef __linux__

#include <X11/Xlib.h>

#endif

typedef struct PlatformInitConfig{
    struct {
        u32 w,h;
    } display;

}PlatformInitConfig;

#ifdef __linux__
typedef struct DisplayProps{
    Atom wmDeleteWindow;
}DisplayProps;
#endif

typedef struct PlatformState{
    struct {
        u32 x,y;
        u32 width, height;

        bool shouldClose;
        enum State{
            Maximized,
            Minimized,
            Normal
        } visibility;


        #ifdef WIN32
        //windows types
        #endif
        #ifdef __linux__
        Display* display;
        Window window;
        DisplayProps __internal;
        #endif
    } display;
    
}PlatformState;

#endif //PLATFORM_TYPES_H