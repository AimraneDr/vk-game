#include <data_types.h>
#include <game.h>

#include <math/vec3.h>

int main(){
    GameState gState = {0};
    GameInitConfig gConfig = {
        .display = {
            .height = 500,
            .width = 750,
            .MaximizeAtStart = false,
            .resizable = true,
            .title = "game"
        },
        .camera = {
            .farPlane = 100.f,
            .nearPlane = 0.01f,
            .fiealdOfView = 45.f,
            .pos = vec3_new(0,0,-1.f),
            .rot = vec3_new(0.f,0.f,0.f)
        }
    };
    game_init(gConfig, &gState);
    
    game_run(&gState);

    game_shutdown(&gState);


    return 0;
}