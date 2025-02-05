#ifndef GAME_H
#define GAME_H

#include "engine_defines.h"
#include "data_types.h"
#include "platform/platform_types.h"
#include "renderer/render_types.h"
#include "core/input.h"
#include "core/clock.h"

typedef struct GameInitConfig{
    struct{
        char* title;
        u16 width, height;
        bool resizable;
        bool MaximizeAtStart;
    }display;
}GameInitConfig;

typedef struct GameState{
    char* name;
    PlatformState platform;
    InputManager inputer;
    Renderer renderer;

    Clock clock;
}GameState;
/**
 * @brief initialize all sub-systems needed by the game
 * @param config init configurations for the game
 * @param out state-ptr for the game
 * @return
 *        RESULT_CODE_SUCCESS : if the initializing succeeded   
 *        RESULT_CODE_FAILED_SYS_INIT : if one or more systems had failed to initialize   
*/
API Result game_init(GameInitConfig config, GameState* out);
API Result game_run(GameState* gState);
API Result game_shutdown(GameState* gState);

#endif //GAME_H