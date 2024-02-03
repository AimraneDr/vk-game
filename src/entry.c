#include "defines.h"

#include <stdio.h>

#include "game/game.h"

int main(){
    GameState gState;
    GameInitConfig gConfig = {
        .display = {
            .height = 500,
            .width = 750,
            .MaximizeAtStart = false,
            .resizable = true,
            .title = "game"
        }
    };
    game_init(gConfig, &gState);

    game_run(&gState);

    game_shutdown(&gState);

    return 0;
}