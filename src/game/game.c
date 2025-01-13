#include "game/game.h"

#include "engine/platform/platform.h"
#include "engine/renderer/renderer.h"

Result game_init(GameInitConfig config, GameState* out){
    // init platform
    PlatformInitConfig info = {
        .display = {
            .w = config.display.width,
            .h = config.display.height
        }
    };


    window_init(info, &out->platform);
    renderer_init(&out->renderer, &out->platform);

    return RESULT_CODE_SUCCESS;
}

Result game_run(GameState* gState){

    while (!gState->platform.display.shouldClose)
    {
        window_PullEvents(&gState->platform);

    }
    return RESULT_CODE_SUCCESS;
}

Result game_shutdown(GameState* gState){

    renderer_shutdown(&gState->renderer);
    window_destroy(&gState->platform);

    return RESULT_CODE_SUCCESS;
}