#ifndef UI_TYPES_H
#define UI_TYPES_H

#include "data_types.h"
#include "renderer/render_types.h"
#include "components/transform.h"
#include <math/mathTypes.h>
#include "ecs/ecs_types.h"
#include "game.h"

typedef struct UI_Style_t {
    //shape
    f32 width,height;

    // Background
    struct {
        Vec4 color;         // RGBA color
        VkDescriptorSet texture; // Optional texture descriptor
        float cornerRadius; // Rounded corners
        Vec4 hoverColor;
    } background;

    // Border
    struct {
        Vec4 color;         // RGBA border color
        float thickness;    // Border width in pixels
        float cornerRadius; // Match background if <0
        Vec4 hoverColor;
    } border;

    // Layout
    Vec4 margin;        // Left, Right, Top, Bottom
    Vec4 padding;       // Left, Right, Top, Bottom

    // Text (if element contains text)
    struct {
        Vec4 color;         // Text color
        float size;         // Font size in points
        Vec2 alignment;     // Horizontal (0=left, 1=right), Vertical (0=top, 1=bottom)
        char* fontName;     // Font asset key
    } text;

    // Interaction states
    Vec4 activeColor;   // Color when clicked
} UI_Style;

#define ACTION_CALLBACK_PTR(name) void (*name)(GameState* state, EntityID e, struct UI_Element_t* elem)
#define ACTION_CALLBACK(name) void name(GameState* state, EntityID e, struct UI_Element_t* elem)

typedef struct UI_Element_t{
    UI_Style style;
    u32 order;             // Render order priority

    

    //TODO: rename to render context
    struct{
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;
        u32 indicesCount;
        bool needsUpdate;  
    }renderer;

    //event callbacks
    ACTION_CALLBACK_PTR(onMouseEnter);
    ACTION_CALLBACK_PTR(onMouseLeave);
    ACTION_CALLBACK_PTR(onMouseStay);
    ACTION_CALLBACK_PTR(onMouseLDown);
    ACTION_CALLBACK_PTR(onMouseRDown);
    ACTION_CALLBACK_PTR(onMouseLUp);
    ACTION_CALLBACK_PTR(onMouseRUp);
    ACTION_CALLBACK_PTR(onMouseLHold);
    ACTION_CALLBACK_PTR(onMouseRHold);

    bool hovered;
} UI_Element;

typedef struct UI_Manager_t{
    UI_Element root;
    f32 pixelsPerPoint;
    bool visible;
}UI_Manager;

#endif //UI_TYPES_H