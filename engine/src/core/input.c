#include "core/input.h"

#include "core/events.h"
#include "core/debugger.h"

#include <collections/DynamicArray.h>
#include <math/vec2.h>

// event's callbacks declaration

void onKeyButtonDown(EventType eType, void *sender, void *listener, EventContext eContext);
void onKeyButtonUp(EventType eType, void *sender, void *listener, EventContext eContext);
void onMouseMove(EventType eType, void *sender, void *listener, EventContext eContext);
void onMouseScroll(EventType eType, void *sender, void *listener, EventContext eContext);

static EventListener
    onKeyDownListener,
    onKeyUpListener,
    onButtonDownListener,
    onButtonUpListener,
    onMouseMoveListener,
    onScrollListener;

void input_system_init(InputManager *manager)
{
    manager->mouse = (MouseState){
        .pos = {0, 0},
        .delta = {0, 0},
        .scrollDelta = 0};

    // subscribe to events
    onKeyDownListener.callback = onKeyButtonDown;
    onKeyDownListener.listener = manager;
    subscribe_to_event(EVENT_TYPE_KEY_DOWN, &onKeyDownListener);

    onKeyUpListener.callback = onKeyButtonUp;
    onKeyUpListener.listener = manager;
    subscribe_to_event(EVENT_TYPE_KEY_UP, &onKeyUpListener);

    onButtonDownListener.callback = onKeyButtonDown;
    onButtonDownListener.listener = manager;
    subscribe_to_event(EVENT_TYPE_MOUSE_BUTTON_DOWN, &onButtonDownListener);

    onButtonUpListener.callback = onKeyButtonUp;
    onButtonUpListener.listener = manager;
    subscribe_to_event(EVENT_TYPE_MOUSE_BUTTON_UP, &onButtonUpListener);

    onMouseMoveListener.callback = onMouseMove;
    onMouseMoveListener.listener = manager;
    subscribe_to_event(EVENT_TYPE_MOUSE_MOVED, &onMouseMoveListener);

    onScrollListener.callback = onMouseScroll;
    onScrollListener.listener = manager;
    subscribe_to_event(EVENT_TYPE_MOUSE_SCROLL, &onScrollListener);
}

void input_system_update(InputManager* manager, f32 deltaTime){
    for(u16 i=0; i<MAX_KEYS; i++){
        if(manager->inputs[i] == KEY_STATE_RELEASED){
            manager->inputs[i] = KEY_STATE_UP;
            manager->pressTime[i] = 0.f;
        } else if (manager->inputs[i] == KEY_STATE_DOWN) {
            manager->inputs[i] = KEY_STATE_PRESSED;
        }
        if(manager->inputs[i] == KEY_STATE_PRESSED){
            manager->pressTime[i] += deltaTime;
        }
    }
    manager->mouse.scrollDelta = 0;
    manager->mouse.delta.dx = 0;
    manager->mouse.delta.dy = 0;
}

void input_system_shutdown(InputManager *manager)
{
    unsubsribe_from_event(EVENT_TYPE_KEY_DOWN, &onKeyDownListener);
    unsubsribe_from_event(EVENT_TYPE_KEY_UP, &onKeyUpListener);
    unsubsribe_from_event(EVENT_TYPE_MOUSE_BUTTON_DOWN, &onButtonDownListener);
    unsubsribe_from_event(EVENT_TYPE_MOUSE_BUTTON_UP, &onButtonUpListener);
    unsubsribe_from_event(EVENT_TYPE_MOUSE_MOVED, &onMouseMoveListener);
    unsubsribe_from_event(EVENT_TYPE_MOUSE_SCROLL, &onScrollListener);
}

bool is_key_down(InputManager* manager, Key key){
    return manager->inputs[key] == KEY_STATE_DOWN;
}
bool is_key_up(InputManager* manager, Key key){
    return manager->inputs[key] == KEY_STATE_UP;
}
bool is_key_pressed(InputManager* manager, Key key){
    return manager->inputs[key] == KEY_STATE_PRESSED;
}
bool is_key_released(InputManager* manager, Key key){
    return manager->inputs[key] == KEY_STATE_RELEASED;
}
f32 get_key_press_duration(InputManager* manager, Key key) {
    return manager->pressTime[key];
}

// event's callbacks definition

void onKeyButtonDown(EventType eType, void *sender, void *listener, EventContext eContext)
{
    if(eContext.u8[0] == KEY_NULL) return;

    InputManager *manager = (InputManager *)listener;
    Key k = eContext.u8[0];
    if(manager->inputs[k] == KEY_STATE_UP || manager->inputs[k] == KEY_STATE_RELEASED){
        manager->inputs[k] = KEY_STATE_DOWN;
    }
};

void onKeyButtonUp(EventType eType, void *sender, void *listener, EventContext eContext)
{
    if(eContext.u8[0] == KEY_NULL) return;
    InputManager *manager = (InputManager *)listener;
    Key k = eContext.u8[0];
    manager->inputs[k] = KEY_STATE_RELEASED;
};

void onMouseMove(EventType eType, void *sender, void *listener, EventContext eContext)
{
    InputManager *manager = (InputManager *)listener;
    u16 newX = eContext.u16[0];
    u16 newY = eContext.u16[1];

    manager->mouse.delta.dx = newX - manager->mouse.pos.x; 
    manager->mouse.delta.dy = newY - manager->mouse.pos.y; 
    manager->mouse.pos.x = newX;
    manager->mouse.pos.y = newY;
};
void onMouseScroll(EventType eType, void *sender, void *listener, EventContext eContext)
{
    InputManager *manager = (InputManager *)listener;
    manager->mouse.scrollDelta += eContext.i16[0];
};

const char *key_name(Key key)
{
    switch (key)
    {
    // Letters
    case KEY_A:
        return "KEY_A";
    case KEY_B:
        return "KEY_B";
    case KEY_C:
        return "KEY_C";
    case KEY_D:
        return "KEY_D";
    case KEY_E:
        return "KEY_E";
    case KEY_F:
        return "KEY_F";
    case KEY_G:
        return "KEY_G";
    case KEY_H:
        return "KEY_H";
    case KEY_I:
        return "KEY_I";
    case KEY_J:
        return "KEY_J";
    case KEY_K:
        return "KEY_K";
    case KEY_L:
        return "KEY_L";
    case KEY_M:
        return "KEY_M";
    case KEY_N:
        return "KEY_N";
    case KEY_O:
        return "KEY_O";
    case KEY_P:
        return "KEY_P";
    case KEY_Q:
        return "KEY_Q";
    case KEY_R:
        return "KEY_R";
    case KEY_S:
        return "KEY_S";
    case KEY_T:
        return "KEY_T";
    case KEY_U:
        return "KEY_U";
    case KEY_V:
        return "KEY_V";
    case KEY_W:
        return "KEY_W";
    case KEY_X:
        return "KEY_X";
    case KEY_Y:
        return "KEY_Y";
    case KEY_Z:
        return "KEY_Z";

    // Numbers
    case KEY_0:
        return "KEY_0";
    case KEY_1:
        return "KEY_1";
    case KEY_2:
        return "KEY_2";
    case KEY_3:
        return "KEY_3";
    case KEY_4:
        return "KEY_4";
    case KEY_5:
        return "KEY_5";
    case KEY_6:
        return "KEY_6";
    case KEY_7:
        return "KEY_7";
    case KEY_8:
        return "KEY_8";
    case KEY_9:
        return "KEY_9";

    // Function Keys
    case KEY_F1:
        return "KEY_F1";
    case KEY_F2:
        return "KEY_F2";
    case KEY_F3:
        return "KEY_F3";
    case KEY_F4:
        return "KEY_F4";
    case KEY_F5:
        return "KEY_F5";
    case KEY_F6:
        return "KEY_F6";
    case KEY_F7:
        return "KEY_F7";
    case KEY_F8:
        return "KEY_F8";
    case KEY_F9:
        return "KEY_F9";
    case KEY_F10:
        return "KEY_F10";
    case KEY_F11:
        return "KEY_F11";
    case KEY_F12:
        return "KEY_F12";

    // Modifiers
    case KEY_SHIFT:
        return "KEY_SHIFT";
    case KEY_CONTROL:
        return "KEY_CONTROL";
    case KEY_ALT:
        return "KEY_ALT";
    case KEY_ESCAPE:
        return "KEY_ESCAPE";
    case KEY_SPACE:
        return "KEY_SPACE";
    case KEY_ENTER:
        return "KEY_ENTER";
    case KEY_BACKSPACE:
        return "KEY_BACKSPACE";
    case KEY_TAB:
        return "KEY_TAB";
    case KEY_CAPS_LOCK:
        return "KEY_CAPS_LOCK";

    // Arrows
    case KEY_UP:
        return "KEY_UP";
    case KEY_DOWN:
        return "KEY_DOWN";
    case KEY_LEFT:
        return "KEY_LEFT";
    case KEY_RIGHT:
        return "KEY_RIGHT";

    // Mouse
    case MOUSE_BUTTON_LEFT:
        return "MOUSE_BUTTON_LEFT";
    case MOUSE_BUTTON_RIGHT:
        return "MOUSE_BUTTON_RIGHT";
    case MOUSE_BUTTON_MIDDLE:
        return "MOUSE_BUTTON_MIDDLE";
    case MOUSE_BUTTON_0:
        return "MOUSE_BUTTON_0";
    case MOUSE_BUTTON_1:
        return "MOUSE_BUTTON_1";

    // Special
    case KEY_NULL:
        return "KEY_NULL";
    case MAX_KEYS:
        return "MAX_KEYS";

    default:
        return "UNKNOWN_KEY";
    }
}