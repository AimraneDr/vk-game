#include "game.h"

#include "platform/platform.h"
#include "renderer/renderer.h"
#include "core/debugger.h"
#include "core/files.h"
#include "core/events.h"
#include "core/input.h"
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
    f64 low = 1000.f, high =0;
    f64 avg=0;
    u32 totalF=0;
    while (!gState->platform.display.shouldClose)
    {
        clock_tick(&gState->clock);

        input_system_update(&gState->inputer);
        renderer_draw(&gState->renderer, &gState->platform, gState->clock.deltaTime);
        window_PullEvents(&gState->platform);

        if(totalF > 0){
            if(low > gState->clock.deltaTime) low = gState->clock.deltaTime;
            if(high < gState->clock.deltaTime) high = gState->clock.deltaTime;
            avg+=gState->clock.deltaTime;
        }
        totalF++;
    }

    LOG_DEBUG("Avg [%.2f fps] | low [%.2f fps] | high [%.2f fps]", 1/(avg/totalF), 1/high, 1/low);
    return RESULT_CODE_SUCCESS;
}

Result game_shutdown(GameState* gState){

    renderer_shutdown(&gState->renderer);
    window_destroy(&gState->platform);

    shutdown_event_sys();
    return RESULT_CODE_SUCCESS;
}