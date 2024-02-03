#include "defines.h"

#include <stdio.h>

#include "platform/platform.h"

int main(){
    DisplayState state = {};
    DisplayInitInfo info = {
        .h = 200,
        .w = 700
    };
    display_init(info, &state);
    while (!state.shouldClose)
    {
        display_PullEvents(&state);

    }

    destroy_display(&state);

    return 0;
}