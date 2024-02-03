#include "platform/platform.h"

#ifdef __linux__
#include <stdio.h>
#include <stdlib.h>


Result display_init(DisplayInitInfo info, DisplayState* out_state){

    out_state->shouldClose = false;
    out_state->width = info.w;
    out_state->height = info.h;

    out_state->display = XOpenDisplay(NULL);
    if(out_state->display == NULL){
        return RESULT_CODE_FAILED_TO_OPEN_DISPLAY;
    }

    int screen = DefaultScreen(out_state->display);

    out_state->window = XCreateWindow(out_state->display, 
                                    DefaultRootWindow(out_state->display),
                                    0,0, 
                                    out_state->width, out_state->height, 0,
                                    DefaultDepth(out_state->display, screen), 
                                    InputOutput,
                                    DefaultVisual(out_state->display, screen), 
                                    0,
                                    NULL);
    
    i64 event_mask = ExposureMask | KeyPressMask | ButtonPressMask |
                    ButtonReleaseMask |  PointerMotionMask | KeyReleaseMask |
                    ResizeRedirectMask; 
    XSelectInput(out_state->display, out_state->window, event_mask);

    XMapWindow(out_state->display, out_state->window);
    XFlush(out_state->display);

    // Set up handling of window close events
    out_state->__internal.wmDeleteWindow = XInternAtom(out_state->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(out_state->display, out_state->window, &out_state->__internal.wmDeleteWindow, 1);


    // Query the actual position of the window
    Window root_return, child_return;
    int root_x, root_y, win_x, win_y;
    unsigned int mask_return;
    XQueryPointer(out_state->display, out_state->window, &root_return, &child_return,
                  &root_x, &root_y, &win_x, &win_y, &mask_return);

    // Update the DisplayState with the actual window position
    out_state->x = win_x;
    out_state->y = win_y;

    // printf("window (x, y) => (%d, %d)\n", win_x, win_y);

    return RESULT_CODE_SUCCESS;
}

Result destroy_display(DisplayState *state){
    XUnmapWindow(state->display, state->window);
    XDestroyWindow(state->display, state->window);
    XCloseDisplay(state->display);

    state->width = 0;
    state->height = 0;
    state->x = 0;
    state->y = 0;
    return RESULT_CODE_SUCCESS;
}


Result display_PullEvents(DisplayState* state){
    while(XPending(state->display)){
        XEvent e; 
        XNextEvent(state->display, &e);
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
            if (e.xclient.message_type == XInternAtom(state->display, "WM_PROTOCOLS", True)) {
                Atom protocol = (Atom)e.xclient.data.l[0];
                if (protocol == state->__internal.wmDeleteWindow) {
                    /* Handle window closing request */
                    state->shouldClose = true;
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