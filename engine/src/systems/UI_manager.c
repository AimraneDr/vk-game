#include "systems/UI_manager.h"

#include "ecs/ecs.h"
#include "core/debugger.h"
#include "components/transform.h"
#include "components/UI/uiComponents.h"
#include "components/UI/details/builder.h"
#include "components/Hierarchy.h"

#include <math/vec4.h>
#include <math/vec2.h>
#include <math/mat.h>
#include <math/mathUtils.h>
#include <collections/DynamicArray.h>

typedef struct UI_Manager_InternalState_t
{
    UI_Element **hovered_elems; // dynamic array
    SystemInfo info;
} UI_Manager_InternalState;

void start(void *sys_state, void *game_state);
void start_entity(void *sys_state, void *game_state, EntityID e);

void pre_update(void *sys_state, void *game_state);
void pre_update_entity(void *sys_state, void *game_state, EntityID e);

void update(void *sys_state, void *game_state);
void update_entity(void *sys_state, void *game_state, EntityID e);

void destroy(void *sys_state, void *game_state);

// internal function
static void checkMouseEvents(GameState *gState, EntityID e, UI_Manager_InternalState *state);
//
// calculations
//
// static void calculateUiElementSize(GameState *gState, EntityID e, UI_Manager_InternalState *state, bool reverse);
// static void calculateUiElementPosition(GameState *gState, EntityID e, UI_Manager_InternalState *state);

System UI_manager_get_system_ref(Scene *scene)
{
    UI_Manager_InternalState *state = malloc(sizeof(UI_Manager_InternalState));
    state->hovered_elems = 0;

    return (System){
        .Signature = COMPONENT_TYPE(scene, Transform2D) | COMPONENT_TYPE(scene, UI_Element),
        .properties = SYSTEM_PROPERTY_HIERARCHY_UPDATE | SYSTEM_PROPERTY_HIERARCHY_PRE_UPDATE,
        .state = state,
        .info = &state->info,
        .callbacks = {
            .start = start,
            .startEntity = start_entity,
            .preUpdate = pre_update,
            .preUpdateEntity = pre_update_entity,
            .update = update,
            .updateEntity = update_entity,
            .destroy = destroy}};
}

void start(void *sys_state, void *game_state)
{
    UI_Manager_InternalState *state = sys_state;
    state->hovered_elems = DynamicArray_Create(UI_Element *);
}
void start_entity(void *sys_state, void *game_state, EntityID e)
{
}

void pre_update(void *sys_state, void *game_state)
{
    Camera *cam = &((GameState *)game_state)->camera;
    ui_canvas_size(vec2_new(cam->viewRect.w, cam->viewRect.h));
    ui_canvas_resolution(cam->pixelsPerPoint);
    ui_canvas_flush();
    ui_builder_reset_tree();
}

void handle_element(EntityID e)
{
    Hierarchy *h = GET_COMPONENT(0, e, Hierarchy);
    UI_Element *elem = GET_COMPONENT(0, e, UI_Element);
    if (elem)
        elem->id = e;
    ui_builder_open_element(e);

    // end element if it has no children
    if (!h || DynamicArray_Length(h->children) == 0)
    {
        ui_builder_close_element();

        // end all parent elemnts that wauts for this element closure
        // the following instructions violates the purpose of the esc design, but i think there is no other choice
        if (h && h->parent != INVALID_ENTITY)
        {
            EntityID currentE = e;
            Hierarchy *currentH = GET_COMPONENT(0, h->parent, Hierarchy);
            while (currentE != INVALID_ENTITY && currentH)
            {
                u32 children_len = DynamicArray_Length(currentH->children);
                if (currentH->children[children_len - 1] != currentE)
                    break;
                ui_builder_close_element();
                currentE = currentH->parent;
                currentH = currentE == INVALID_ENTITY ? 0 : GET_COMPONENT(0, currentE, Hierarchy);
            }
        }
    }
}

void pre_update_entity(void *sys_state, void *game_state, EntityID e)
{
    // the setup pass
    handle_element(e);
}

void update(void *sys_state, void *game_state)
{
    // the calculations pass
    ui_builder_build();
}

void update_entity(void *sys_state, void *game_state, EntityID e)
{

    UI_Manager_InternalState *state = sys_state;
    InputManager *inputs = &((GameState *)game_state)->inputer;

    UI_Element *elem = GET_COMPONENT(0, e, UI_Element);
    Hierarchy *h = GET_COMPONENT(0, e, Hierarchy);

    // TODO: add elem checking, even if it is hovered that does not mean it is what is selected, a child elem can be selected instead
    checkMouseEvents(game_state, e, sys_state);

    // fire click events for top most elem
    if (elem->hovered)
    {
        bool topMost = true;
        u32 hovered_count = DynamicArray_Length(state->hovered_elems);
        for (u8 i = 0; i < hovered_count; i++)
        {
            EntityID id = state->hovered_elems[i]->id;
            Hierarchy *_h = GET_COMPONENT(0, id, Hierarchy);
            if (_h && _h->depth_level > h->depth_level)
            {
                topMost = false;
                break;
            }
        }
        if (topMost)
        {
            if (is_key_down(inputs, MOUSE_BUTTON_LEFT))
            {
                elem->mouse_state.Lclicked = true;
                if (elem->events.onMouseLDown)
                    elem->events.onMouseLDown(game_state, e, elem);
            }
            if (is_key_up(inputs, MOUSE_BUTTON_LEFT))
            {
                elem->mouse_state.Lclicked = false;
                if (elem->events.onMouseLUp)
                    elem->events.onMouseLUp(game_state, e, elem);
            }

            if (is_key_down(inputs, MOUSE_BUTTON_RIGHT))
            {
                elem->mouse_state.Rclicked = true;
                if (elem->events.onMouseRDown)
                    elem->events.onMouseRDown(game_state, e, elem);
            }
            if (is_key_up(inputs, MOUSE_BUTTON_RIGHT))
            {
                elem->mouse_state.Rclicked = false;
                if (elem->events.onMouseRUp)
                    elem->events.onMouseRUp(game_state, e, elem);
            }
        }
    }

    if (elem->mouse_state.Lclicked && is_key_pressed(inputs, MOUSE_BUTTON_LEFT) && elem->events.onMouseLHold)
        elem->events.onMouseLHold(game_state, e, elem);
    if (elem->mouse_state.Rclicked && is_key_pressed(inputs, MOUSE_BUTTON_RIGHT) && elem->events.onMouseRHold)
        elem->events.onMouseRHold(game_state, e, elem);
}

void destroy(void *sys_state, void *game_state)
{
    UI_Manager_InternalState *state = sys_state;
    DynamicArray_Destroy(state->hovered_elems);
}

///////////////////////////////
///////////////////////////////
/////      INTERNAL     ///////
///////////////////////////////
///////////////////////////////

typedef struct onMouseMoveData_t
{
    EntityID e;
    UI_Manager_InternalState *state;
} onMouseMoveData;

/*
    these checks depends on the size and position of the elements,
    so they need to be performed after all sizing & layout calculations are done
    (hince : post-update)
*/
void checkMouseEvents(GameState *gState, EntityID e, UI_Manager_InternalState *state)
{
    InputManager *inputs = &gState->inputer;
    Scene *scene = &gState->scene;

    UI_Element *elem = GET_COMPONENT(scene, e, UI_Element);

    if (inputs->mouse.pos.x == -1)
    {
        elem->hovered = false;
        return;
    }

    Transform2D *transform = GET_COMPONENT(scene, e, Transform2D);
    Vec4 mouse_screen = vec4_new(inputs->mouse.pos.x, inputs->mouse.pos.y, 0, 1);
    Vec4 mouse_local = vec4_zero();

    Mat4 model_matrix = mat4_mul(transform->mat, transform->__local_mat);

    // Correct inverse for hit detection:
    Mat4 inv_model = mat4_inverse(model_matrix);
    mouse_local = mat4_mulVec4(inv_model, mouse_screen);

    Vec4 size = vec4_fromVec2(ui_element_get_final_size(elem), 0, 1);
    if (mouse_local.x >= 0 &&
        mouse_local.x <= size.x &&
        mouse_local.y >= 0 &&
        mouse_local.y <= size.y)
    {
        if (!elem->hovered)
        {
            elem->hovered = true;
            bool present_in_hovered = false;
            for (u8 i = 0; i < DynamicArray_Length(state->hovered_elems); i++)
            {
                if (state->hovered_elems[i]->id == elem->id)
                {
                    present_in_hovered = true;
                    break;
                }
            }
            if (!present_in_hovered)
                DynamicArray_Push(state->hovered_elems, elem);
            if (elem->events.onMouseEnter)
                elem->events.onMouseEnter(gState, e, elem);
            return;
        }
        else
        {
            if (elem->events.onMouseStay)
                elem->events.onMouseStay(gState, e, elem);
            return;
        }
    }

    if (elem->hovered)
    {
        elem->hovered = false;
        if (elem->events.onMouseLeave)
            elem->events.onMouseLeave(gState, e, elem);
        for (u8 i = 0; i < DynamicArray_Length(state->hovered_elems); i++)
        {
            if (state->hovered_elems[i]->id == elem->id)
            {
                DynamicArray_PopAt(state->hovered_elems, i, 0);
            }
        }
        return;
    }
}

// void calculateUiElementSize(GameState *gState, EntityID e, UI_Manager_InternalState *state, bool reverse)
// {
//     Scene *scene = &gState->scene;

//     UI_Element *elem = GET_COMPONENT(scene, e, UI_Element);
//     if (reverse)
//     {
//         // leaf -> root
//         switch (elem->style.size)
//         {
//         case UI_SIZE_FIT:
//             Hierarchy *h = GET_COMPONENT(scene, e, Hierarchy);
//             if (!h || DynamicArray_Length(h->children) == 0)
//             {
//                 elem->internal.final.width = elem->style.padding.right + elem->style.padding.left;
//                 elem->internal.final.height = elem->style.padding.top + elem->style.padding.bottom;
//             }
//             else
//             {
//                 elem->internal.final.width = 0;
//                 elem->internal.final.height = 0;
//                 for (u16 i = 0; i < DynamicArray_Length(h->children); i++)
//                 {
//                     EntityID child = h->children[i];
//                     UI_Element *childElem = GET_COMPONENT(scene, child, UI_Element);
//                     // TODO: ADD PADDING & fix layout dir
//                     elem->internal.final.width += childElem->internal.final.width;
//                     elem->internal.final.height += childElem->internal.final.height;
//                 }
//             }
//             break;
//         }
//     }
//     else
//     {
//         // root -> leaf
//         switch (elem->style.size)
//         {
//         case UI_SIZE_FULL:
//             Hierarchy *h = GET_COMPONENT(scene, e, Hierarchy);
//             if (h && h->parent != INVALID_ENTITY)
//             {
//                 UI_Element *parentElem = GET_COMPONENT(scene, h->parent, UI_Element);
//                 elem->internal.final.width = parentElem->internal.final.width;
//                 elem->internal.final.height = parentElem->internal.final.height;
//             }
//             else
//             {
//                 elem->internal.final.width = gState->camera.viewRect.x / gState->camera.pixelsPerPoint;
//                 elem->internal.final.height = gState->camera.viewRect.y / gState->camera.pixelsPerPoint;
//             }
//             break;
//         case UI_SIZE_WIDTH_HEIGHT:
//             elem->internal.final.width = elem->style.width;
//             elem->internal.final.height = elem->style.height;
//             break;
//         }
//     }
// }
// void calculateUiElementPosition(GameState *gState, EntityID e, UI_Manager_InternalState *state)
// {
//     // root -> leaf
//     Scene *scene = &gState->scene;
//     Camera *cam = &gState->camera;

//     UI_Element *elem = GET_COMPONENT(scene, e, UI_Element);
//     Transform2D *t = GET_COMPONENT(scene, e, Transform2D);

//     // root elements first
//     Hierarchy *h = GET_COMPONENT(scene, e, Hierarchy);
//     switch (elem->style.horizontalAlignement)
//     {
//     case UI_ALIGNEMENT_START:
//         t->position.x = 0; // elem->internal.final.width / 2.f;
//         break;
//     case UI_ALIGNEMENT_CENTER:
//         if (!h || h->parent == INVALID_ENTITY)
//         {
//             t->position.x = cam->viewRect.x / cam->pixelsPerPoint / 2.f;
//         }
//         else
//         {
//             UI_Element *parentElem = GET_COMPONENT(scene, h->parent, UI_Element);
//             t->position.x = parentElem->internal.final.width / 2.f;
//         }
//         break;
//     case UI_ALIGNEMENT_END:
//         if (!h || h->parent == INVALID_ENTITY)
//         {
//             t->position.x = (cam->viewRect.x / cam->pixelsPerPoint) - (elem->internal.final.width / 2.f);
//         }
//         else
//         {
//             UI_Element *parentElem = GET_COMPONENT(scene, h->parent, UI_Element);
//             t->position.x = parentElem->internal.final.width - (elem->internal.final.width / 2.f);
//         }
//         break;
//     }

//     switch (elem->style.verticalAlignement)
//     {
//     case UI_ALIGNEMENT_START:
//         t->position.y = 0; // elem->__.final.height / 2.f;
//         break;
//     case UI_ALIGNEMENT_CENTER:
//         if (!h || h->parent == INVALID_ENTITY)
//         {
//             t->position.y = cam->viewRect.y / cam->pixelsPerPoint / 2.f;
//         }
//         else
//         {
//             UI_Element *parentElem = GET_COMPONENT(scene, h->parent, UI_Element);
//             t->position.y = parentElem->internal.final.height / 2.f;
//         }
//         break;
//     case UI_ALIGNEMENT_END:
//         if (!h || h->parent == INVALID_ENTITY)
//         {
//             t->position.y = (cam->viewRect.y / cam->pixelsPerPoint) - (elem->internal.final.height / 2.f);
//         }
//         else
//         {
//             UI_Element *parentElem = GET_COMPONENT(scene, h->parent, UI_Element);
//             t->position.y = parentElem->internal.final.height - (elem->internal.final.height / 2.f);
//         }
//         break;
//     }
// }
