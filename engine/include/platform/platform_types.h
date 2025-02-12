#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

#include "data_types.h"
#include <string/str_types.h>

#ifdef __linux__

#include <X11/Xlib.h>

#endif

#ifdef _WIN32

#include <windows.h>

#endif

#ifdef __linux__
typedef struct DisplayProps{
    Atom wmDeleteWindow;
}DisplayProps;
#endif

typedef enum WindowState{
    WINDOW_STATE_NULL,
    WINDOW_STATE_FULL_SCREEN,
    WINDOW_STATE_MAXIMIZED,
    WINDOW_STATE_FLOATING,
    WINDOW_STATE_MINIMIZED
}WindowState;

typedef enum WindowStartPosition_e{
    WINDOW_START_POSITION_RANDOM = -1,
    WINDOW_START_POSITION_CENTER = -2,
    WINDOW_START_POSITION_END = -3,
    WINDOW_START_POSITION_START = 0,
}WindowStartPosition;

typedef struct PlatformInitConfig{
    struct {
        u32 w,h;
        i32 x,y;
        bool resizable;
        WindowState startState;
    } display;
    String title;
}PlatformInitConfig;

typedef struct PlatformState{
    struct Window{
        String title;
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