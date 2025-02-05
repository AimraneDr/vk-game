#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

#include "engine/data_types.h"

#ifdef __linux__

#include <X11/Xlib.h>

#endif

#ifdef _WIN32

#include <windows.h>

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

typedef enum WindowState{
    Maximized,
    Minimized,
    Floating
}WindowState;

typedef struct PlatformState{
    struct Window{
        u32 x,y;
        u32 width, height;

        bool shouldClose;
        WindowState visibility;

        bool isResizing;

        #ifdef _WIN32
        HWND hwnd;
        HINSTANCE hInstance;
        #endif
        #ifdef __linux__
        Display* display;
        Window window;
        DisplayProps __internal;
        #endif
    } display;
    
}PlatformState;

#endif //PLATFORM_TYPES_H