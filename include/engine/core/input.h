#ifndef INPUT_H
#define INPUT_H

#include "engine/data_types.h"

typedef enum KeyState{
    KEY_STATE_UP,
    KEY_STATE_DOWN,
    KEY_STATE_PRESSED
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
    MOUSE_BUTTON_2,
    
    MAX_KEYS
}Key;

typedef struct InputState_t{
    Key key;
    KeyState state;
}InputState;

typedef struct MouseState_t{
    struct {
        u16 x,y;
    }pos;
    i16 scroll;
}MouseState;

typedef struct InputManager{
    /// @brief a dynamic list of keys that possess an action
    InputState* inputs;
    MouseState mouse;
}InputManager;

void input_system_init(InputManager* manager);
void input_system_update(InputManager* manager);
void input_system_shutdown(InputManager* manager);

/// @brief whene a key is first pressed
/// @param manager 
/// @param key 
/// @return 
bool is_key_down(InputManager* manager, Key key);
/// @brief whene a key is released
/// @param manager 
/// @param key 
/// @return 
bool is_key_up(InputManager* manager, Key key);
/// @brief whene a key is held down
/// @param manager 
/// @param key 
/// @return 
bool is_key_pressed(InputManager* manager, Key key);


const char* key_name(Key key);
#endif //INPUT_H