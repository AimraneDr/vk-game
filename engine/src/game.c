#include "game.h"

#include "platform/platform.h"
#include "renderer/renderer.h"
#include "core/debugger.h"
#include "core/files.h"
#include "core/events.h"
#include "core/input.h"
#include "assets/asset_manager.h"
#include <math/mathTypes.h>
#include <collections/DynamicArray.h>

#include <math/vec3.h>


void game_shutdown(GameState* state);
void GameInitConfigSetDefaults(GameConfig* config);



void game_init(GameConfig config, GameState* out){
    // init platform
    out->name = str_new(config.platform.title.val);
    PlatformInitConfig info = {
        .display = {
            .x = config.platform.display.x,
            .y = config.platform.display.y,
            .w = config.platform.display.w,
            .h = config.platform.display.h,
            .resizable = config.platform.display.resizable,
            .startState = config.platform.display.startState,
        },
        .title = out->name
    };

    out->camera.transform.position = (Vec3){
        .x = config.camera.pos.x,
        .y = config.camera.pos.y,
        .z = config.camera.pos.z,
    };
    out->camera.transform.rotation = (Vec3){
        .x = config.camera.rot.x,
        .y = config.camera.rot.y,
        .z = config.camera.rot.z,
    };
    out->camera.transform.scale = (Vec3){
        .x = config.camera.scale.x,
        .y = config.camera.scale.y,
        .z = config.camera.scale.z,
    };
    out->camera.farPlane = config.camera.farPlane;
    out->camera.nearPlane = config.camera.nearPlane;
    out->camera.fieldOfView = config.camera.fieldOfView;
    out->camera.orthographicSize = config.camera.orthographicSize;
    out->camera.useOrthographic = config.camera.useOrthographic;

    init_event_sys();

    window_init(info, &out->platform);
    input_system_init(&out->inputer);
    asset_manager_init(&out->assetManager);

    renderer_init(config.renderer, &out->renderer, &out->platform);

    out->meshRenderers = DynamicArray_Create(MeshRenderer);

    return;
}

void game_run(GameInterface Interface){
    GameState state = {0};
    GameConfig gConfig = {0};

    if(Interface.config) Interface.config(&gConfig);
    
    GameInitConfigSetDefaults(&gConfig);
    
    game_init(gConfig, &state);

    //user specific startup
    if(Interface.start) Interface.start(&state);

    clock_start(&state.clock);
    while (!state.platform.display.shouldClose)
    {
        clock_tick(&state.clock);

        input_system_update(&state.inputer, state.clock.deltaTime);

        renderer_draw(&state.camera, &state.renderer, &state.platform, state.meshRenderers, state.clock.deltaTime, &state.uiManager);
        
        window_PullEvents(&state.platform);
        
        if(Interface.update) Interface.update(&state);
        if(is_key_down(&state.inputer, KEY_P)){
            LOG_DEBUG("%.2f FPS", 1 /state.clock.deltaTime);
        }
    }

    if(Interface.cleanup) Interface.cleanup(&state);
    
    game_shutdown(&state);
    return;
}

void game_shutdown(GameState* state){

    DynamicArray_Destroy(state->meshRenderers);

    asset_manager_shutdown(&state->assetManager);
    renderer_shutdown(&state->renderer);
    window_destroy(&state->platform);

    shutdown_event_sys();
    str_free(&state->platform.display.title);
    return;
}


void GameInitConfigSetDefaults(GameConfig* config) {
    GameConfig base = {
        .platform = {
            .display = {
                .h = 500,
                .w = 750,
                .startState = WINDOW_STATE_FLOATING,
            },
            .title = str_new("game")
        },
        .camera = {
            .farPlane = 100.f,
            .nearPlane = 0.01f,
            .fieldOfView = 45.f,
            .orthographicSize = 1.f,
            .pos = vec3_new(0.f,1.f,0.f),
            .rot = vec3_new(0.f,0.f,0.f),
            .scale = vec3_new(1.f,1.f,1.f)
        }
    };
    // Set display defaults
    if (config->platform.title.len == 0) {
        config->platform.title = str_new(base.platform.title.val);
    }
    if (config->platform.display.w == 0) {
        config->platform.display.w = base.platform.display.w;
    }
    if (config->platform.display.h == 0) {
        config->platform.display.h = base.platform.display.h;
    }
    if (config->platform.display.startState == WINDOW_STATE_NULL) {
        config->platform.display.startState = base.platform.display.startState;
    }
    if (config->camera.pos.x == 0.0f && config->camera.pos.y == 0.0f && config->camera.pos.z == 0.0f) {
        config->camera.pos = (Vec3){base.camera.pos.x, base.camera.pos.y, base.camera.pos.z};
    }
    if (config->camera.scale.x == 0.0f && config->camera.scale.y == 0.0f && config->camera.scale.z == 0.0f) {
        config->camera.scale = (Vec3){base.camera.scale.x, base.camera.scale.y, base.camera.scale.z};
    }
    if (config->camera.orthographicSize == 0.0f) {
        config->camera.orthographicSize = base.camera.orthographicSize;
    }
    if (config->camera.fieldOfView == 0.0f) {
        config->camera.fieldOfView = base.camera.fieldOfView;
    }
    if (config->camera.farPlane == 0.0f) {
        config->camera.farPlane = base.camera.farPlane;
    }
    if (config->camera.nearPlane == 0.0f) {
        config->camera.nearPlane = base.camera.nearPlane;
    }
}