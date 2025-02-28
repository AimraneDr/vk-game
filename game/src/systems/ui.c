#include "systems/ui.h"

#include <components/transform.h>
#include <components/UI/ui_types.h>
#include <components/characterController.h>
#include <ecs/ecs.h>
#include <core/debugger.h>
#include <components/UI/uiComponents.h>
#include <math/vec4.h>
#include<math/mathUtils.h>
#include <core/input.h>
#include <game.h>
#include <core/events.h>
#include <math/vec2.h>

typedef struct InternalState_t{
    
}InternalState;


static void start(void* _state,  void* gState);
static void update_entity(void* _state, void* gState, EntityID entity);
static void destroy_entity(void* _state,  void* gState, EntityID entity);

System game_ui_system_ref(Scene* scene, Renderer* r){
    return (System){
        .Signature = COMPONENT_TYPE(scene, Transform2D) | COMPONENT_TYPE(scene, UI_Element),
        .state = 0,
        .properties = SYSTEM_PROPERTY_HIERARCHY_UPDATE,
        .start = start,
        .updateEntity = update_entity,
        .destroyEntity = destroy_entity
    };
}

static ACTION_CALLBACK(_onMouseLHold){
    Transform2D* t = GET_COMPONENT(&state->scene, e, Transform2D);
    Vec2 delta = vec2_new(state->inputer.mouse.delta.dx/state->camera.pixelsPerPoint, state->inputer.mouse.delta.dy/state->camera.pixelsPerPoint);
    if(delta.x != 0 || delta.y != 0){
        t->position = vec2_add(t->position, delta);
    }
}

void start(void* _state,  void* gState){
    Renderer* r = &((GameState*)gState)->renderer;
    Scene* scene = &((GameState*)gState)->scene;
    Camera* camera = &((GameState*)gState)->camera;
    CharacterController controller = {
        .MoveSpped = 5.f,
        .RotateSpeed = 45.f
    };
    Transform2D containerTransform = {
        .position = {0.f, .5f},
        .scale = {1.f, 1.f},
        .rotation = 0.f,
    };
    UI_Style containerStyle = {
        .background.color = vec4_new(0,.7,.4,1),
        .background.hoverColor = vec4_new(.5,.7,.4,1),
        .width =1.f,
        .height =1.f,
    };
    UI_Style containerStyle0 = {
        .background.color = vec4_new(0,.4,.7,1),
        .background.hoverColor = vec4_new(.5,.4,.7,1),
        .width =5.f,
        .height =2.5f,
    };
    UI_Style containerStyle1 = {
        .background.color = vec4_new(1,.5,.5,1),
        .background.hoverColor = vec4_new(1,.1,.1,1),
        .width =1.f,
        .height =1.f,
    };
    UI_Element c1 = ui_create_container(containerStyle, r);
    UI_Element c2 = ui_create_container(containerStyle0, r);
    UI_Element c3 = ui_create_container(containerStyle1, r);
    
    c1.onMouseLHold = _onMouseLHold;
    EntityID container = newEntity(scene);
    ADD_COMPONENT(scene, container, UI_Element, &c1);
    ADD_COMPONENT(scene, container, Transform2D, &containerTransform);
    EntityID container0 = newEntity(scene);
    ecs_add_child(scene, container0, container);
    containerTransform.position = (Vec2){0,0};
    containerTransform.scale = (Vec2){2,2};
    ADD_COMPONENT(scene, container0, UI_Element, &c2);
    ADD_COMPONENT(scene, container0, Transform2D, &containerTransform);
    
    EntityID container1 = newEntity(scene);
    containerTransform.position = (Vec2){.2,.3};
    containerTransform.scale = (Vec2){1.3,.5};
    ADD_COMPONENT(scene, container1, UI_Element, &c3);
    ADD_COMPONENT(scene, container1, Transform2D, &containerTransform);
    ecs_add_child(scene, container, container1);
    
    ADD_COMPONENT(scene, container0, CharacterController, &controller);
}

static void update_entity(void* _state,  void* gState, EntityID entity){

    InputManager* inputs = &((GameState*)gState)->inputer;
    Camera* camera = &((GameState*)gState)->camera;
    f64 dt = ((GameState*)gState)->clock.deltaTime;
    UI_Element* E = GET_COMPONENT(&((GameState*)gState)->scene, entity, UI_Element);
    Transform2D* T = GET_COMPONENT(&((GameState*)gState)->scene, entity, Transform2D);
    CharacterController* C = GET_COMPONENT(&((GameState*)gState)->scene, entity, CharacterController);
    if(C){
        
        if(is_key_pressed(inputs, KEY_A)){
            T->position.x -= C->MoveSpped * dt;
        }
        if(is_key_pressed(inputs, KEY_D)){
            T->position.x += C->MoveSpped * dt;
        }
        if(is_key_pressed(inputs, KEY_W)){
            T->position.y -= C->MoveSpped * dt;
        }
        if(is_key_pressed(inputs, KEY_S)){
            T->position.y += C->MoveSpped * dt;
        }

        if(is_key_pressed(inputs, KEY_Q)){
            T->rotation += C->RotateSpeed * dt;
        }
        if(is_key_pressed(inputs, KEY_E)){
            T->rotation -= C->RotateSpeed * dt;
        }

        if(is_key_pressed(inputs, KEY_R)){
            T->scale.x += C->MoveSpped * dt;
            T->scale.y += C->MoveSpped * dt;
        }
        if(is_key_pressed(inputs, KEY_F)){
            T->scale.x -= C->MoveSpped * dt;
            T->scale.y -= C->MoveSpped * dt;
        }
        f32 step = 1.f;
        //RED
        if(is_key_pressed(inputs, KEY_1) && is_key_pressed(inputs, KEY_SHIFT)){
            E->style.background.color.x -= step*dt;
            CLAMP(E->style.background.color.x, 0, 1);
        }else if(is_key_pressed(inputs, KEY_1)){
            E->style.background.color.x += step*dt;
            CLAMP(E->style.background.color.x, 0, 1);
        }
        //BLUE
        if(is_key_pressed(inputs, KEY_2) && is_key_pressed(inputs, KEY_SHIFT)){
            E->style.background.color.y -= step*dt;
            CLAMP(E->style.background.color.y, 0, 1);
        }else if(is_key_pressed(inputs, KEY_2)){
            E->style.background.color.y += step*dt;
            CLAMP(E->style.background.color.y, 0, 1);
        }
        //GREEM
        if(is_key_pressed(inputs, KEY_3) && is_key_pressed(inputs, KEY_SHIFT)){
            E->style.background.color.z -= step*dt;
            CLAMP(E->style.background.color.z, 0, 1);
        }else if(is_key_pressed(inputs, KEY_3)){
            E->style.background.color.z += step*dt;
            CLAMP(E->style.background.color.z, 0, 1);
        }
        //ALPHA
        if(is_key_pressed(inputs, KEY_4) && is_key_pressed(inputs, KEY_SHIFT)){
            E->style.background.color.w -= step*dt;
            CLAMP(E->style.background.color.w, 0, 1);
        }else if(is_key_pressed(inputs, KEY_4)){
            E->style.background.color.w += step*dt;
            CLAMP(E->style.background.color.w, 0, 1);
        }
    }
}

void destroy_entity(void* _state,  void* gState, EntityID entity){
    InternalState* state = _state;
    Scene* scene = &((GameState*)gState)->scene;
    Renderer* r = &((GameState*)gState)->renderer;

    UI_Element* elem = GET_COMPONENT(scene, entity, UI_Element);
    if(elem != 0){
        ui_destroyElement(elem, r);
    }
}