#include <data_types.h>

#include <game.h>
#include <core/files.h>
#include <core/debugger.h>
#include <core/input.h>
#include <assets/asset_manager.h>
#include <core/files.h>
#include <core/events.h>
#include <ecs/ecs.h>
#include <components/meshRenderer.h>
#include <components/UI/uiComponents.h>
#include <collections/DynamicArray.h>

#include <math/trigonometry.h>
#include <math/vec2.h>
#include <math/vec4.h>
#include <math/mat.h>

#include <math/vec3.h>

#include "systems/ui.h"

void config(GameConfig* config){
    
    config->platform.display.h = 500;
    config->platform.display.w = 750;
    config->platform.display.x = WINDOW_START_POSITION_CENTER;
    config->platform.display.y = WINDOW_START_POSITION_CENTER;
    config->platform.display.startState = WINDOW_STATE_FLOATING; //FIXME: WINDOW_STATE_MINIMIZED breaks the renderer;
    config->platform.display.resizable = true;
    config->platform.title = str_new("My App");
    
    config->camera.farPlane = 1000.f;
    config->camera.nearPlane = 0.01f;
    config->camera.fieldOfView = 60.f;
    config->camera.pos = vec3_new(0.f,5.f,-5.f);
    config->camera.rot = vec3_new(45.f,0.f,0.f);
    config->camera.orthographicSize = 3.f;
    config->camera.useOrthographic = true;

    config->renderer.msaaSamples = 4;

    config->suspendOnMinimize = true;
}

void start(GameState* state){
    state->camera.pixelsPerPoint = 50.f;
    System sys = game_ui_system_ref(&state->scene, &state->renderer);
    ecs_register_system(&state->scene, &sys);


    Asset a = load_asset(&state->assetManager, "./../resources/models/viking_room.obj");
    Asset a1 = load_asset(&state->assetManager, "./../resources/models/cube.obj");
    MeshRenderer mesh0,mesh1;
    createMeshRenderer(a.data, &state->renderer, &mesh0);
    createMeshRenderer(a1.data, &state->renderer, &mesh1);
    release_asset(&state->assetManager, &a);
    release_asset(&state->assetManager, &a1);

    //test ecs entity
    EntityID vikingRoom0 = newEntity(&state->scene);
    EntityID cube = newEntity(&state->scene);
    Transform initT = {
        .position = {1,0,1},
        .rotation = {45,0,0},
        .scale = {1,1,1}
    };
    ADD_COMPONENT(&state->scene, vikingRoom0, Transform, &initT);
    ADD_COMPONENT(&state->scene, vikingRoom0, MeshRenderer, &mesh0);
    initT.position = vec3_new(-1,0,-1);
    ADD_COMPONENT(&state->scene, cube, Transform, &initT);
    ADD_COMPONENT(&state->scene, cube, MeshRenderer, &mesh1);

}

static void update(GameState* state){
    if(is_key_down(&state->inputer, KEY_O)){
        state->camera.useOrthographic=!state->camera.useOrthographic;
    }

    if(is_key_down(&state->inputer, KEY_C)){
        LOG_DEBUG("camera rect : %.2f x %.2f", state->camera.viewRect.x,state->camera.viewRect.y);
    }

    static f32 changeSpeed = .5;
    if(state->inputer.mouse.scrollDelta != 0){
        if(state->camera.useOrthographic){
            state->camera.orthographicSize += state->inputer.mouse.scrollDelta * changeSpeed;
        }else{
            state->camera.fieldOfView += state->inputer.mouse.scrollDelta * changeSpeed;
        }
    }
}
static void cleanup(GameState* state){
}

int main(){
    GameInterface Interface;
    Interface.config = config;
    Interface.start = start;
    Interface.update = update;
    Interface.cleanup = cleanup;
    game_run(Interface);
}