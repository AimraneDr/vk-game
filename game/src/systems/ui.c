#include "systems/ui.h"

#include <components/transform.h>
#include <components/UI/ui_types.h>
#include <components/characterController.h>
#include <ecs/ecs.h>
#include <core/debugger.h>
#include <components/UI/uiComponents.h>
#include <math/vec4.h>
#include <math/mathUtils.h>
#include <core/input.h>
#include <game.h>
#include <core/events.h>
#include <math/vec2.h>
#include <math/mat.h>

typedef struct InternalState_t
{

} InternalState;

static void start(void *_state, void *gState);
static void update_entity(void *_state, void *gState, EntityID entity);
static void destroy_entity(void *_state, void *gState, EntityID entity);

System game_ui_system_ref(Scene *scene, Renderer *r)
{
    return (System){
        .Signature = COMPONENT_TYPE(scene, Transform2D) | COMPONENT_TYPE(scene, UI_Element),
        .state = 0,
        .properties = SYSTEM_PROPERTY_HIERARCHY_UPDATE,
        .callbacks = {
            .start = start,
            .updateEntity = update_entity,
            .destroyEntity = destroy_entity}};
}

static ACTION_CALLBACK(_onMouseLHold)
{
    Transform2D *t = GET_COMPONENT(&state->scene, e, Transform2D);
    Vec2 delta = vec2_new(
        state->inputer.mouse.delta.dx,
        state->inputer.mouse.delta.dy);

    if (delta.x != 0 || delta.y != 0)
    {
        // If the element has a parent, adjust delta for parent's transform
        if (!mat4_compare(t->__local_mat, mat4_identity()))
        {
            // TODO: move the space convertion to a helper function
            Mat4 inv_parent = mat4_inverse(t->__local_mat);
            Vec4 transformed_delta = mat4_mulVec4(inv_parent, vec4_fromVec2(delta, 0, 0));
            delta = vec2_fromVec4(transformed_delta);
        }

        t->position = vec2_add(t->position, delta);
    }
}
static ACTION_CALLBACK(_onMouseDown)
{
    Transform2D *t = GET_COMPONENT(&state->scene, e, Transform2D);
    
}
#include <collections/DynamicArray.h>
void start(void *_state, void *gState)
{
    Renderer *r = &((GameState *)gState)->renderer;
    Scene *scene = &((GameState *)gState)->scene;

    // CharacterController controller = {
    //     .MoveSpped = 5.f,
    //     .RotateSpeed = 45.f
    // };
    Transform2D containerTransform = {
        .position = vec2_new(1.f, 1.f),
        .scale = vec2_new(1.f, 1.f),
        .rotation = 0.f,
    };
    UI_Style containerStyle = {
        .background.color = vec4_new(.1, .1, .1, 1),
        .background.hoverColor = vec4_new(.1, .3, .1, 1),
        .width = 25.f,
        .height = 25.f,
        .border = {
            .color = vec4_new(0, 1, 1, 1),
            .hoverColor = vec4_new(0, .5, .5, 1),
            .thickness = 1000},
        .layout = UI_LAYOUT_HORIZONTAL,
        .padding = vec4_new(12, 12, 12, 12),
        .gap = vec2_new(12, 12),
        .margin = vec4_zero(),
    };
    UI_Style textStyle = {
        .background.color = vec4_new(1, 1, 1, 1),
        .background.hoverColor = vec4_new(1, .1, .1, 1),
        .padding = vec4_new(12, 12, 12, 12),
        .size = UI_SIZE_FIT,
        .text = {
            .fontName = "dejavu",
            .size = 32.f,
        }
    };

    UI_Element c = ui_create_container(containerStyle);
    c.events.onMouseLDown = _onMouseDown;
    EntityID container = newEntity(scene); // 96
    ADD_COMPONENT(scene, container, UI_Element, &c);
    ADD_COMPONENT(scene, container, Transform2D, &containerTransform);
    
    UI_Element text = ui_create_text(textStyle, "Aimrane Draou");
    EntityID textE = newEntity(scene);
    ADD_COMPONENT(scene, textE, UI_Element, &text);
    ADD_COMPONENT(scene, textE, Transform2D, &containerTransform);

    ecs_move_entity(0, container, textE);
    ui_canvas_padding(vec4_new(12, 12, 12, 12));
    // ADD_COMPONENT(scene, container0, CharacterController, &controller);
}

static void update_entity(void *_state, void *gState, EntityID entity)
{

    InputManager *inputs = &((GameState *)gState)->inputer;

    f64 dt = ((GameState *)gState)->clock.deltaTime;
    UI_Element *E = GET_COMPONENT(&((GameState *)gState)->scene, entity, UI_Element);
    Transform2D *T = GET_COMPONENT(&((GameState *)gState)->scene, entity, Transform2D);
    CharacterController *C = GET_COMPONENT(&((GameState *)gState)->scene, entity, CharacterController);
    if (C)
    {

        if (is_key_pressed(inputs, KEY_A))
        {
            T->position.x -= C->MoveSpped * dt;
        }
        if (is_key_pressed(inputs, KEY_D))
        {
            T->position.x += C->MoveSpped * dt;
        }
        if (is_key_pressed(inputs, KEY_W))
        {
            T->position.y -= C->MoveSpped * dt;
        }
        if (is_key_pressed(inputs, KEY_S))
        {
            T->position.y += C->MoveSpped * dt;
        }

        if (is_key_pressed(inputs, KEY_Q))
        {
            T->rotation += C->RotateSpeed * dt;
        }
        if (is_key_pressed(inputs, KEY_E))
        {
            T->rotation -= C->RotateSpeed * dt;
        }

        if (is_key_pressed(inputs, KEY_R))
        {
            T->scale.x += C->MoveSpped * dt;
            T->scale.y += C->MoveSpped * dt;
        }
        if (is_key_pressed(inputs, KEY_F))
        {
            T->scale.x -= C->MoveSpped * dt;
            T->scale.y -= C->MoveSpped * dt;
        }
        f32 step = 1.f;
        // RED
        if (is_key_pressed(inputs, KEY_1) && is_key_pressed(inputs, KEY_SHIFT))
        {
            E->style.background.color.x -= step * dt;
            E->style.background.color.x = CLAMP(E->style.background.color.x, 0, 1);
        }
        else if (is_key_pressed(inputs, KEY_1))
        {
            E->style.background.color.x += step * dt;
            E->style.background.color.x = CLAMP(E->style.background.color.x, 0, 1);
        }
        // BLUE
        if (is_key_pressed(inputs, KEY_2) && is_key_pressed(inputs, KEY_SHIFT))
        {
            E->style.background.color.y -= step * dt;
            E->style.background.color.y = CLAMP(E->style.background.color.y, 0, 1);
        }
        else if (is_key_pressed(inputs, KEY_2))
        {
            E->style.background.color.y += step * dt;
            E->style.background.color.y = CLAMP(E->style.background.color.y, 0, 1);
        }
        // GREEM
        if (is_key_pressed(inputs, KEY_3) && is_key_pressed(inputs, KEY_SHIFT))
        {
            E->style.background.color.z -= step * dt;
            E->style.background.color.z = CLAMP(E->style.background.color.z, 0, 1);
        }
        else if (is_key_pressed(inputs, KEY_3))
        {
            E->style.background.color.z += step * dt;
            E->style.background.color.z = CLAMP(E->style.background.color.z, 0, 1);
        }
        // ALPHA
        if (is_key_pressed(inputs, KEY_4) && is_key_pressed(inputs, KEY_SHIFT))
        {
            E->style.background.color.w -= step * dt;
            E->style.background.color.w = CLAMP(E->style.background.color.w, 0, 1);
        }
        else if (is_key_pressed(inputs, KEY_4))
        {
            E->style.background.color.w += step * dt;
            E->style.background.color.w = CLAMP(E->style.background.color.w, 0, 1);
        }
    }
}

void destroy_entity(void *_state, void *gState, EntityID entity)
{
    // InternalState* state = _state;
    Scene *scene = &((GameState *)gState)->scene;
    Renderer *r = &((GameState *)gState)->renderer;

    UI_Element *elem = GET_COMPONENT(scene, entity, UI_Element);
    if (elem != 0)
    {
        ui_destroy_element(elem, r);
    }
}