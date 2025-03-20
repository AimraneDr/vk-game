#ifndef INPUT_H
#define INPUT_H

#include "data_types.h"
#include "engine_defines.h"

typedef enum KeyState{
    KEY_STATE_UP,
    KEY_STATE_DOWN,
    KEY_STATE_PRESSED,
    KEY_STATE_RELEASED
}KeyState;

typedef enum Key{
    KEY_NULL = 0,
    
    // Letters
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    
    // Numbers
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    
    // Function Keys
    KEY_F1 ,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    
    // Modifiers
    KEY_SHIFT,
    KEY_CONTROL,
    KEY_ALT,
    KEY_ESCAPE,
    KEY_SPACE,
    KEY_ENTER,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_CAPS_LOCK,
    
    // Arrows
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    
    // Mouse
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_MIDDLE,
    MOUSE_BUTTON_0,
    MOUSE_BUTTON_1,
    
    MAX_KEYS
}Key;

typedef struct MouseState_t{
    struct {
        i16 x,y;
    }pos;
    struct {
        i16 dx,dy;
    }delta;
    i16 scrollDelta;
}MouseState;

typedef struct InputManager{
    KeyState inputs[MAX_KEYS];
    f32 pressTime[MAX_KEYS];
    MouseState mouse;
}InputManager;

void input_system_init(InputManager* manager);
void input_system_update(InputManager* manager, f32 deltaTime);
void input_system_shutdown(InputManager* manager);

/// @brief whene a key is first pressed
API bool is_key_down(InputManager* manager, Key key);

/// @brief check if key is not pressed
API bool is_key_up(InputManager* manager, Key key);

/// @brief whene a key is held down
API bool is_key_pressed(InputManager* manager, Key key);

/// @brief whene a key is first released
API bool is_key_released(InputManager* manager, Key key);

/// @brief get the duration a key has been pressed
API f32 get_key_press_duration(InputManager* manager, Key key);


API const char* key_name(Key key);
#endif //INPUT_H