#include <data_types.h>

#include <game.h>
#include <core/files.h>
#include <core/debugger.h>
#include <core/input.h>
#include <assets/asset_manager.h>
#include <core/files.h>
#include <core/events.h>
#include <components/meshRenderer.h>
#include <components/UI/uiComponents.h>
#include <collections/DynamicArray.h>

#include <math/trigonometry.h>
#include <math/vec2.h>
#include <math/vec4.h>
#include <math/mat.h>

#include <math/vec3.h>


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
    state->uiManager.pixelsPerPoint = 50.f;

    Asset a = load_asset(&state->assetManager, "./../resources/models/viking_room.obj");
    Asset a1 = load_asset(&state->assetManager, "./../resources/models/cube.obj");
    MeshRenderer mesh0,mesh1;
    createMeshRenderer(a.data, &state->renderer, &mesh0);
    createMeshRenderer(a1.data, &state->renderer, &mesh1);
    DynamicArray_Push(state->meshRenderers, mesh0);
    DynamicArray_Push(state->meshRenderers, mesh1);
    release_asset(&state->assetManager, &a);
    release_asset(&state->assetManager, &a1);

    Transform2D containerTransform = {
        .position = {5.f, 3.f},
        .scale = {1.f, 1.5f},
        .rotation = 90.f,
        .pivot = {.5f ,.5f}
    };
    UI_Style containerStyle = {
        .background.color = vec4_new(1,0,0,1)
    };
    ui_createRootElement(&state->uiManager);
    UI_Element* c1 = ui_create_container(&state->uiManager.root, containerTransform, containerStyle, &state->renderer);
    containerTransform.position = vec2_new(3, 5);
    containerStyle.background.color = vec4_new(0,1,0,1);
    UI_Element* c2 = ui_create_container(c1, containerTransform, containerStyle, &state->renderer);

    containerTransform.position = vec2_new(2, 2);
    containerStyle.background.color = vec4_new(0,0,1,1);
    ui_create_container(c2, containerTransform, containerStyle, &state->renderer);
    containerTransform.position = vec2_new(7, 2);
    containerStyle.background.color = vec4_new(0,0,1,1);
    ui_create_container(c2, containerTransform, containerStyle, &state->renderer);
}

void update(GameState* state){
    if(is_key_down(&state->inputer, KEY_O)){
        state->camera.useOrthographic=!state->camera.useOrthographic;
    }
    static f32 changeSpeed = .5;
    if(state->inputer.mouse.scrollDelta != 0){
        if(state->camera.useOrthographic){
            state->camera.orthographicSize += state->inputer.mouse.scrollDelta * changeSpeed;
        }else{
            state->camera.fieldOfView += state->inputer.mouse.scrollDelta * changeSpeed;
        }
    }
    static float angle = 0;
    angle += state->clock.deltaTime * deg_to_rad(90.0f);
    i8 dir = 1;
    f32 scale = 1.f;
    for(u32 i=0; i < DynamicArray_Length(state->meshRenderers); i++){
        state->meshRenderers[i].mat4 = mat4_mul(
            mat4_scaling((Vec3){scale, scale, scale}),
            mat4_mul(
                mat4_rotation(angle, (Vec3){.0f, 1.0f, .0f}),
                mat4_translation((Vec3){dir * 1.f, 0.f, dir * 1.f})
            )
        );
        dir*=-1;
        scale  = scale == 1.f ? 1.5f : 1.f;
    }

    state->uiManager.root.children[0].transform.rotation+= 45.0f * state->clock.deltaTime;
}
void cleanup(GameState* state){
    u32 length = DynamicArray_Length(state->meshRenderers);
    for(u32 i=0; i < length; i++){
        destroyMeshRenderer(&state->renderer, &state->meshRenderers[i]);
    }

    ui_destroyElement(&state->uiManager.root, &state->renderer);
}

int main(){
    GameInterface Interface;
    Interface.config = config;
    Interface.start = start;
    Interface.update = update;
    Interface.cleanup = cleanup;
    game_run(Interface);
}