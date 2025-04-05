#ifndef UI_TYPES_H
#define UI_TYPES_H

#include "data_types.h"
#include "renderer/render_types.h"
#include "components/transform.h"
#include <math/mathTypes.h>
#include "ecs/ecs_types.h"
#include "game.h"


typedef enum UI_Element_Size_Mode_e{
    UI_SIZE_UNIT_POINT,
    UI_SIZE_UNIT_PIXEL,
    UI_SIZE_UNIT_PERCENT,
}UISizeUnit;

typedef enum UI_Element_Size_e{
    UI_SIZE_FIT = 1,
    UI_SIZE_FULL,
    UI_SIZE_WIDTH_HEIGHT
}UISize;

typedef enum UI_Element_Layout_e{
    UI_LAYOUT_HORIZONTAL,
    UI_LAYOUT_VERTICAL
}UILayout;

typedef enum UI_Box_e{
    UI_BOX_FLEX,
    UI_BOX_GRID
}UIBox;

typedef struct UI_Style_t {
    //shape
    f32 width,height;


    UISize size;
    UISizeUnit size_unit;
    UILayout layout;
    UIBox box;
    
    // Background
    struct {
        Vec4 color;         // RGBA color
        VkDescriptorSet sprite; // Optional texture descriptor
        Vec4 hoverColor;
    } background;

    // Border
    struct {
        Vec4 color;         // RGBA border color
        f32 thickness;    // Border width in pixels
        f32 cornerRadius; // Match background if <0
        Vec4 hoverColor;
    } border;

    // Layout
    Vec4 margin;        // top, Right, bottom, left
    Vec4 padding;       // top, Right, bottom, left
    Vec2 gap;           // horizontal, vertical

    // Text (if element contains text)
    struct {
        Vec4 color;         // Text color
        f32 size;         // Font size in points
        Vec2 alignment;     // Horizontal (0=left, 1=right), Vertical (0=top, 1=bottom)
        char* fontName;     // Font asset key
    } text;

    // Interaction states
    Vec4 activeColor;   // Color when clicked
} UI_Style;

#define ACTION_CALLBACK_PTR(name) void (*name)(GameState* state, EntityID e, struct UI_Element_t* elem)
#define ACTION_CALLBACK(name) void name(GameState* state, EntityID e, struct UI_Element_t* elem)

typedef struct UI_Element_t{
    EntityID id;
    UI_Style style;
    u32 order;             // Render order priority

    

    //TODO: move or change
    struct{
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;
        u32 indicesCount;
        bool needsUpdate;  
    }renderer;

    //event callbacks
    struct{
        ACTION_CALLBACK_PTR(onMouseEnter);
        ACTION_CALLBACK_PTR(onMouseLeave);
        ACTION_CALLBACK_PTR(onMouseStay);
        ACTION_CALLBACK_PTR(onMouseLDown);
        ACTION_CALLBACK_PTR(onMouseRDown);
        ACTION_CALLBACK_PTR(onMouseLUp);
        ACTION_CALLBACK_PTR(onMouseRUp);
        ACTION_CALLBACK_PTR(onMouseLHold);
        ACTION_CALLBACK_PTR(onMouseRHold);
    }events;
    struct {
        bool Lclicked;
        bool Rclicked;
        bool Mclicked;
        bool B0clicked;
        bool B1clicked;
    }mouse_state;

    bool hovered;

    void* internal;
} UI_Element;

typedef struct UI_Manager_t{
    UI_Element root;
    f32 pixelsPerPoint;
    bool visible;
}UI_Manager;

#endif //UI_TYPES_H