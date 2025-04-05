#include <data_types.h>

#include <game.h>
#include <core/debugger.h>
#include <core/input.h>
#include <assets/asset_manager.h>
#include <ecs/ecs.h>
#include <components/meshRenderer.h>
#include <components/UI/uiComponents.h>
#include <string/str.h>

#include <math/trigonometry.h>
#include <math/vec2.h>
#include <math/vec4.h>
#include <math/mat.h>
#include <math/mathUtils.h>

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
    config->camera.nearPlane = 0.0001f;
    config->camera.fieldOfView = 65.f;
    config->camera.pos = vec3_new(0.f,5.f,-5.f);
    config->camera.rot = vec3_new(45.f,0.f,0.f);
    config->camera.orthographicSize = 3.f;
    config->camera.useOrthographic = false;

    config->renderer.msaaSamples = 4;

    config->suspendOnMinimize = true;

    config->targetFPS = 180;
}

void createDefaultMaterial(Material* out){
    Asset* _default = get_asset("default", ASSET_TYPE_TEXTURE);    
    out->albedo = _default->data;
    out->height = _default->data;
    out->emissive = _default->data;
    out->metalRoughAO = _default->data;
    out->normal = _default->data;
    out->normal = _default->data;
    out->uvOffset = vec2_zero();
    out->uvTiling = vec2_one();
}

void start(GameState* state){
    state->camera.pixelsPerPoint = 12.f;
    System sys = game_ui_system_ref(&state->scene, &state->renderer);
    ecs_register_system(&state->scene, &sys);


    load_asset("./../resources/textures/viking_room.png", "viking_room");
    load_asset("./../resources/models/viking_room.obj", "viking_room");
    load_asset("./../resources/models/plane.obj", "plane");

    MeshRenderer viking_mesh,plane, plane0;
    createMeshRenderer("viking_room", &viking_mesh);
    createMeshRenderer("plane", &plane);
    createMeshRenderer("plane", &plane0);
    
    //TODO: temp
    Asset* texture = get_asset("image", ASSET_TYPE_TEXTURE);
    Asset* viking_tex = get_asset("viking_room", ASSET_TYPE_TEXTURE);
    
    createDefaultMaterial(viking_mesh.material);
    createDefaultMaterial(plane0.material);
    createDefaultMaterial(plane.material);

    viking_mesh.material->albedo = viking_tex->data ? viking_tex->data : viking_mesh.material->albedo;
    plane.material->albedo = texture->data? texture->data : plane.material->albedo;
    
    plane0.material->uvTiling = vec2_new(2,2);
    //end temp

    //test ecs entity
    EntityID vikingRoom0 = newEntity(&state->scene);
    EntityID cube = newEntity(&state->scene);
    EntityID planeE = newEntity(&state->scene);
    Transform initT = {
        .position = vec3_new(0,0.5,0),
        .rotation = vec3_new(0,0,0),
        .scale = vec3_new(1,1,1)
    };
    ADD_COMPONENT(&state->scene, vikingRoom0, Transform, &initT);
    ADD_COMPONENT(&state->scene, vikingRoom0, MeshRenderer, &viking_mesh);
    initT.position = vec3_new(0,1.5,1);
    initT.rotation = vec3_new(90,0,0);
    initT.scale = vec3_new(3,0,3);
    ADD_COMPONENT(&state->scene, cube, Transform, &initT);
    ADD_COMPONENT(&state->scene, cube, MeshRenderer, &plane);
    
    initT.position = vec3_new(0,0,-.5);
    initT.rotation = vec3_new(0,0,0);
    initT.scale = vec3_new(3,0,3);
    ADD_COMPONENT(&state->scene, planeE, Transform, &initT);
    ADD_COMPONENT(&state->scene, planeE, MeshRenderer, &plane0);
}

void updateCamController(GameState* state) {
    Camera* cam = &state->camera;
    static f32 moveSpeed = 5.f, rotateSpeed = 30.f;
    
    Vec3 movement = vec3_zero();
    Vec3 forward = camera_forward(cam);
    Vec3 up = camera_up(cam);
    Vec3 right = camera_right(cam);

    // Handle movement differently for orthographic vs perspective
    if (cam->useOrthographic) {
        // Orthographic controls (2D-like movement)
        if (is_key_pressed(&state->inputer, KEY_W)) {
            movement = vec3_add(movement, vec3_scale(up, moveSpeed * state->clock.deltaTime));
        }
        if (is_key_pressed(&state->inputer, KEY_S)) {
            movement = vec3_add(movement, vec3_scale(up, -moveSpeed * state->clock.deltaTime));
        }
        if (is_key_pressed(&state->inputer, KEY_A)) {
            movement = vec3_add(movement, vec3_scale(right, -moveSpeed * state->clock.deltaTime));
        }
        if (is_key_pressed(&state->inputer, KEY_D)) {
            movement = vec3_add(movement, vec3_scale(right, moveSpeed * state->clock.deltaTime));
        }
        // Q/E for depth (optional, might not affect orthographic view)
        if (is_key_pressed(&state->inputer, KEY_Q)) {
            movement = vec3_add(movement, vec3_scale(forward, moveSpeed * state->clock.deltaTime));
        }
        if (is_key_pressed(&state->inputer, KEY_E)) {
            movement = vec3_add(movement, vec3_scale(forward, -moveSpeed * state->clock.deltaTime));
        }
    } else {
        // Perspective controls (original code)
        if (is_key_pressed(&state->inputer, KEY_W)) {
            movement = vec3_add(movement, vec3_scale(forward, moveSpeed * state->clock.deltaTime));
        }
        if (is_key_pressed(&state->inputer, KEY_S)) {
            movement = vec3_add(movement, vec3_scale(forward, -moveSpeed * state->clock.deltaTime));
        }
        if (is_key_pressed(&state->inputer, KEY_A)) {
            movement = vec3_add(movement, vec3_scale(right, -moveSpeed * state->clock.deltaTime));
        }
        if (is_key_pressed(&state->inputer, KEY_D)) {
            movement = vec3_add(movement, vec3_scale(right, moveSpeed * state->clock.deltaTime));
        }
        if (is_key_pressed(&state->inputer, KEY_Q)) {
            movement = vec3_add(movement, vec3_scale(up, moveSpeed * state->clock.deltaTime));
        }
        if (is_key_pressed(&state->inputer, KEY_E)) {
            movement = vec3_add(movement, vec3_scale(up, -moveSpeed * state->clock.deltaTime));
        }
    }

    // Apply movement to camera position
    cam->transform.position = vec3_add(cam->transform.position, movement);

    // Handle rotation (optional: disable rotation for orthographic)
    if (!cam->useOrthographic) { // Only allow rotation in perspective mode
        if (is_key_pressed(&state->inputer, KEY_UP)) {
            cam->transform.rotation.x -= rotateSpeed * state->clock.deltaTime;
        }
        if (is_key_pressed(&state->inputer, KEY_DOWN)) {
            cam->transform.rotation.x += rotateSpeed * state->clock.deltaTime;
        }
        if (is_key_pressed(&state->inputer, KEY_RIGHT)) {
            cam->transform.rotation.y += rotateSpeed * state->clock.deltaTime;
        }
        if (is_key_pressed(&state->inputer, KEY_LEFT)) {
            cam->transform.rotation.y -= rotateSpeed * state->clock.deltaTime;
        }

        const float maxPitch = 85.f;
        cam->transform.rotation.x = CLAMP(cam->transform.rotation.x, -maxPitch, maxPitch);
    }
}

static void update(GameState* state){
    if(is_key_down(&state->inputer, KEY_O)){
        state->camera.useOrthographic=!state->camera.useOrthographic;
    }

    if(is_key_down(&state->inputer, KEY_C)){
        LOG_DEBUG("camera rect : %.2f x %.2f", state->camera.viewRect.x,state->camera.viewRect.y);
    }

    updateCamController(state);

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