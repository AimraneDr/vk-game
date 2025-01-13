#include "engine/platform/platform.h"

#ifdef __linux__
#include <stdio.h>
#include <stdlib.h>


Result window_init(PlatformInitConfig info, PlatformState* out){

    out->display.shouldClose = false;
    out->display.width = info.display.w;
    out->display.height = info.display.h;

    out->display.display = XOpenDisplay(NULL);
    if(out->display.display == NULL){
        return RESULT_CODE_FAILED_DISPLAY_OPENING;
    }

    int screen = DefaultScreen(out->display.display);

    out->display.window = XCreateWindow(out->display.display, 
                                    DefaultRootWindow(out->display.display),
                                    0,0, 
                                    out->display.width, out->display.height, 0,
                                    DefaultDepth(out->display.display, screen), 
                                    InputOutput,
                                    DefaultVisual(out->display.display, screen), 
                                    0,
                                    NULL);
    
    i64 event_mask = ExposureMask | KeyPressMask | ButtonPressMask |
                    ButtonReleaseMask |  PointerMotionMask | KeyReleaseMask |
                    ResizeRedirectMask; 
    XSelectInput(out->display.display, out->display.window, event_mask);

    XMapWindow(out->display.display, out->display.window);
    XFlush(out->display.display);

    // Set up handling of window close events
    out->display.__internal.wmDeleteWindow = XInternAtom(out->display.display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(out->display.display, out->display.window, &out->display.__internal.wmDeleteWindow, 1);


    // Query the actual position of the window
    Window root_return, child_return;
    int root_x, root_y, win_x, win_y;
    unsigned int mask_return;
    XQueryPointer(out->display.display, out->display.window, &root_return, &child_return,
                  &root_x, &root_y, &win_x, &win_y, &mask_return);

    // Update the PlatformState with the actual window position
    out->display.x = win_x;
    out->display.y = win_y;

    // printf("window (x, y) => (%d, %d)\n", win_x, win_y);

    return RESULT_CODE_SUCCESS;
}

Result window_destroy(PlatformState *state){
    XUnmapWindow(state->display.display, state->display.window);
    XDestroyWindow(state->display.display, state->display.window);
    XCloseDisplay(state->display.display);

    state->display.width = 0;
    state->display.height = 0;
    state->display.x = 0;
    state->display.y = 0;
    return RESULT_CODE_SUCCESS;
}


Result window_PullEvents(PlatformState* state){
    while(XPending(state->display.display)){
        XEvent e; 
        XNextEvent(state->display.display, &e);
        switch (e.type)
        {
        case Expose:
            /* code */
            break;
        case KeyPress:

            break;
        case KeyRelease:

            break;
        case ButtonPress:

            break;
        case ButtonRelease:

            break;
        case MotionNotify:

            break;
        case ResizeRequest:

            break;
        case UnmapNotify :
            break;
        case ClientMessage:
            if (e.xclient.message_type == XInternAtom(state->display.display, "WM_PROTOCOLS", True)) {
                Atom protocol = (Atom)e.xclient.data.l[0];
                if (protocol == state->display.__internal.wmDeleteWindow) {
                    /* Handle window closing request */
                    state->display.shouldClose = true;
                }
            }
            break;
        default:

            break;
        }
    }
    return RESULT_CODE_SUCCESS;
}

#endif