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

typedef struct InternalState_t{
    
}InternalState;

static void start(void* _state,  void* gState);
static void update_entity(void* _state, void* gState, EntityID entity);
static void destroy_entity(void* _state,  void* gState, EntityID entity);

System game_ui_system_ref(Scene* scene, Renderer* r){
    return (System){
        .Signature = COMPONENT_TYPE(scene, Transform2D) | COMPONENT_TYPE(scene, UI_Element),
        .state = 0,
        .start = start,
        .updateEntity = update_entity,
        .destroyEntity = destroy_entity
    };
}

void start(void* _state,  void* gState){
    Renderer* r = &((GameState*)gState)->renderer;
    Scene* scene = &((GameState*)gState)->scene;
    CharacterController controller = {
        .MoveSpped = 5.f,
        .RotateSpeed = 45.f
    };
    Transform2D containerTransform = {
        .position = {0.f, 4.f},
        .scale = {1.f, 1.f},
        .rotation = 90.f,
    };
    UI_Style containerStyle = {
        .background.color = vec4_new(1,0,0,1)
    };
    UI_Style containerStyle0 = {
        .background.color = vec4_new(0,1,0,1)
    };
    UI_Element c1 = ui_create_container(containerStyle, r);
    UI_Element c2 = ui_create_container(containerStyle0, r);
    EntityID container = newEntity(scene);
    ADD_COMPONENT(scene, container, UI_Element, &c1);
    ADD_COMPONENT(scene, container, Transform2D, &containerTransform);
    ADD_COMPONENT(scene, container, CharacterController, &controller);
    EntityID container0 = newEntity(scene);
    containerTransform.position = (Vec2){0,0};
    containerTransform.scale = (Vec2){1,1};
    ADD_COMPONENT(scene, container0, UI_Element, &c2);
    ADD_COMPONENT(scene, container0, Transform2D, &containerTransform);
}

static void update_entity(void* _state,  void* gState, EntityID entity){
    InputManager* inputs = &((GameState*)gState)->inputer;
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