#ifndef GAME_H
#define GAME_H

#include "game_types.h"
#include "engine/platform/platform_types.h"

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
}GameState;
/**
 * @brief initialize all sub-systems needed by the game
 * @param config init configurations for the game
 * @param out state-ptr for the game
 * @return
 *        RESULT_CODE_SUCCESS : if the initializing succeeded   
 *        RESULT_CODE_FAILED_SYS_INIT : if one or more systems had failed to initialize   
*/
Result game_init(GameInitConfig config, GameState* out);
Result game_run(GameState* gState);
Result game_shutdown(GameState* gState);

#endif //GAME_H