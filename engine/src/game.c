#include "game.h"

#include "platform/platform.h"
#include "renderer/renderer.h"
#include "core/debugger.h"
#include "core/memsys.h"
#include "core/events.h"
#include "core/input.h"
#include "assets/asset_manager.h"
#include <math/mathTypes.h>
#include <math/vec3.h>
#include <math/vec2.h>
#include <string/str.h>

#include "components/Hierarchy.h"
#include "components/transform.h"
#include "components/meshRenderer.h"
#include "components/characterController.h"

#include "systems/PBR_renderer.h"
#include "systems/UI_renderer.h"
#include "systems/UI_manager.h"

#include "ecs/ecs.h"

EVENT_CALLBACK(onMinimize){
    GameState* state = (GameState*)listener;
    state->suspended = true;
}

EVENT_CALLBACK(onActivated){
    GameState* state = (GameState*)listener;
    state->suspended = false;
}

EVENT_CALLBACK(onWiinResize){
    Camera* camera = (Camera*)listener;
    PlatformState* p = sender;

    camera->viewRect.x = p->display.width;
    camera->viewRect.y = p->display.height;
}

void game_shutdown(GameState* state);
void GameInitConfigSetDefaults(GameConfig* config);
void RegisterDefaultComponents(Scene* scene);
void RegisterDefaultSystems(Scene* scene, Renderer* r, Camera* camera);

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
    out->targetFrameTime = config.targetFPS ? 1.0 / config.targetFPS : 0.0;
    
    memsys_init();
    init_event_sys();
    
    window_init(info, &out->platform);
    
    input_system_init(&out->inputer);
    asset_manager_init();
    asset_manager_load_default_loaders();

    out->camera.viewRect = vec2i_new(out->platform.display.width,out->platform.display.height);
    //ecs
    ecs_init(out);
    ecs_set_active_scene(&out->scene);
    RegisterDefaultComponents(&out->scene);
    RegisterDefaultSystems(&out->scene, &out->renderer, &out->camera);

    renderer_init(config.renderer, out);

    return;
}

void game_run(GameInterface Interface){
    GameState state = {0};
    GameConfig gConfig = {0};

    if(Interface.config) Interface.config(&gConfig);
    
    GameInitConfigSetDefaults(&gConfig);


    game_init(gConfig, &state);
    
    if(gConfig.suspendOnMinimize){
        EventListener listner = {
            .callback = onMinimize,
            .listener = &state
        };
        subscribe_to_event(EVENT_TYPE_WINDOW_MINIMIZED, &listner);
        listner.callback = onActivated;
        subscribe_to_event(EVENT_TYPE_WINDOW_ACTIVATED, &listner);
    } 
    subscribe_to_event(
        EVENT_TYPE_WINDOW_RESIZED,
        &(EventListener){
            .callback = onWiinResize,
            .listener = &state.camera
        }
    );
    //user specific startup
    if(Interface.start) Interface.start(&state);

    ecs_systems_start_group(&state, 0, SYSTEM_GROUP_UI);
    ecs_systems_initialize(&state, 0);

    Clock frame_timer;
    clock_start(&state.clock);
    clock_start(&frame_timer);
    while (!state.platform.display.shouldClose)
    {
        clock_tick(&frame_timer);
        window_PullEvents(&state.platform);
        if(!state.suspended){
            clock_tick(&state.clock);
            
            ecs_systems_pre_update(&state, 0);
            ecs_systems_pre_update_group(&state, 0, SYSTEM_GROUP_UI);
            ecs_systems_update_group(&state, 0, SYSTEM_GROUP_UI);
            renderer_draw(&state);
            
            if(Interface.update) Interface.update(&state);
            if(is_key_down(&state.inputer, KEY_P)){
                LOG_DEBUG("%.2f FPS", 1 /state.clock.deltaTime);
            }
            ecs_systems_update(&state, 0);
            ecs_systems_post_update_group(&state, 0, SYSTEM_GROUP_UI);
            ecs_systems_post_update(&state, 0);
        }
        ecs_update(0);
        
        input_system_update(&state.inputer, state.clock.deltaTime);
        clock_tick(&frame_timer);
        if (frame_timer.deltaTime < state.targetFrameTime)
        {
            platform_sleep(state.targetFrameTime - frame_timer.deltaTime);
        }
    }

    ecs_systems_shutdown(&state, 0);
    ecs_systems_destroy_group(&state, 0, SYSTEM_GROUP_UI);
    if(Interface.cleanup) Interface.cleanup(&state);
    
    game_shutdown(&state);
    return;
}

void game_shutdown(GameState* state){

    asset_manager_shutdown();
    renderer_shutdown(state);
    ecs_shutdown(&state->scene);
    window_destroy(&state->platform);

    shutdown_event_sys();
    memsys_shutdown();
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
        },
        .targetFPS = 0
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
        config->camera.pos = vec3_new(base.camera.pos.x, base.camera.pos.y, base.camera.pos.z);
    }
    if (config->camera.scale.x == 0.0f && config->camera.scale.y == 0.0f && config->camera.scale.z == 0.0f) {
        config->camera.scale = vec3_new(base.camera.scale.x, base.camera.scale.y, base.camera.scale.z);
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

void RegisterDefaultComponents(Scene* scene){
    REGIATER_COMPONENT(scene, Hierarchy);
    REGIATER_COMPONENT(scene, Transform);
    REGIATER_COMPONENT(scene, Transform2D);
    REGIATER_COMPONENT(scene, MeshRenderer);
    REGIATER_COMPONENT(scene, UI_Element);
    REGIATER_COMPONENT(scene, CharacterController);
}

void RegisterDefaultSystems(Scene* scene, Renderer* r, Camera* camera){
    System pbr_renderer = PBR_renderer_get_system_ref(scene,r, camera);
    System ui_renderer = UI_renderer_get_system_ref(scene,r);
    System ui_manager = UI_manager_get_system_ref(scene);
    ecs_register_system_to_group(scene, &pbr_renderer, SYSTEM_GROUP_RENDERING);
    ecs_register_system_to_group(scene, &ui_renderer, SYSTEM_GROUP_RENDERING);
    ecs_register_system_to_group(scene, &ui_manager, SYSTEM_GROUP_UI);
}