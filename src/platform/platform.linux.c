#include "platform/platform.h"

#ifdef __linux__
#include <stdio.h>
#include <xcb/xcb.h>

Result display_init(DisplayInitInfo info, DisplayState* out_state){
    xcb_screen_t *screen;
    int screen_num;

    out_state->width = info.w;
    out_state->hright = info.h;
    out_state->connection = xcb_connect(NULL, &screen_num);
    if (!out_state->connection) {
        //messages to be handled in an Error Manager
        printf("Unable to make an XCB connection\n");
        return RESULT_CODE_FAILED_TO_MAKE_XCB_CONX;
    }

    const xcb_setup_t *setup = xcb_get_setup(out_state->connection);

    xcb_screen_iterator_t iter = xcb_setup_roots_iterator (setup);  

    // we want the screen at index screenNum of the iterator
    for (u32 i = 0; i < screen_num; i++) {
        xcb_screen_next (&iter);
    }

    screen = iter.data;

    out_state->x = (screen->width_in_pixels / 2) - (info.w / 2);
    out_state->y = (screen->height_in_pixels / 2) - (info.h / 2);


    out_state->window = xcb_generate_id(out_state->connection);
    uint32_t value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t value_list[2] = {screen->black_pixel, XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS};

    xcb_create_window(out_state->connection, XCB_COPY_FROM_PARENT, out_state->window, screen->root, 0, 0, info.w, info.h, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, value_mask, value_list);

    xcb_map_window(out_state->connection, out_state->window);
    xcb_flush(out_state->connection);
    return RESULT_CODE_SUCCESS;
}

Result destroy_display(DisplayState *state){
    // Unmap and destroy the window
    xcb_unmap_window(state->connection, state->window);
    xcb_destroy_window(state->connection, state->window);

    // Disconnect from the X server
    xcb_disconnect(state->connection);

    state->connection = null;
    state->window = null;
    state->width = 0;
    state->hright = 0;
    state->x = 0;
    state->y = 0;

    return RESULT_CODE_SUCCESS;
}

#endif