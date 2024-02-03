#include "game/game.h"

#include "engine/platform/platform.h"

Result game_init(GameInitConfig config, GameState* out){
    // init platform
    PlatformInitConfig info = {
        .display = {
            .w = config.display.width,
            .h = config.display.height
        }
    };
    display_init(info, &out->platform);

    return RESULT_CODE_SUCCESS;
}
Result game_run(GameState* gState){

    while (!gState->platform.display.shouldClose)
    {
        display_PullEvents(&gState->platform);

    }
    return RESULT_CODE_SUCCESS;
}
Result game_shutdown(GameState* gState){

    destroy_display(&gState->platform);

    return RESULT_CODE_SUCCESS;
}