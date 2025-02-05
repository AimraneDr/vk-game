#include "game/game.h"

#include "engine/platform/platform.h"
#include "engine/renderer/renderer.h"
#include "engine/core/debugger.h"
#include "engine/core/files.h"
#include "engine/core/events.h"
#include "engine/core/input.h"
#include <math/mathTypes.h>

void on_window_resized(EventType eType, void* sender, void* listener, EventContext eContext){
    EventContextWindowResize context =  CAST_EVENT_CONTEXT(eContext, EventContextWindowResize);
    LOG_INFO("Window resized to %d x %d", context.width, context.height);
}

EventListener onWinResizeListener = {
    .callback = on_window_resized
};

Result game_init(GameInitConfig config, GameState* out){
    // init platform
    PlatformInitConfig info = {
        .display = {
            .w = config.display.width,
            .h = config.display.height
        }
    };

    clock_start(&out->clock);
    init_event_sys();

    window_init(info, &out->platform);
    input_system_init(&out->inputer);
    renderer_init(&out->renderer, &out->platform);


    subscribe_to_event(EVENT_TYPE_WINDOW_RESIZING, &onWinResizeListener);

    return RESULT_CODE_SUCCESS;
}

Result game_run(GameState* gState){

    while (!gState->platform.display.shouldClose)
    {
        clock_tick(&gState->clock);

        input_system_update(&gState->inputer);
        renderer_draw(&gState->renderer, &gState->platform, gState->clock.deltaTime);
        window_PullEvents(&gState->platform);
    }
    return RESULT_CODE_SUCCESS;
}

Result game_shutdown(GameState* gState){

    renderer_shutdown(&gState->renderer);
    window_destroy(&gState->platform);

    shutdown_event_sys();
    return RESULT_CODE_SUCCESS;
}