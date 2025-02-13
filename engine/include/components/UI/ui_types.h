#ifndef UI_TYPES_H
#define UI_TYPES_H

#include "data_types.h"
#include "renderer/render_types.h"
#include "components/transform.h"
#include <math/mathTypes.h>

typedef struct UI_Style_t {
    // Background
    struct {
        Vec4 color;         // RGBA color
        VkDescriptorSet texture; // Optional texture descriptor
        float cornerRadius; // Rounded corners
    } background;

    // Border
    struct {
        Vec4 color;         // RGBA border color
        float thickness;    // Border width in pixels
        float cornerRadius; // Match background if <0
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
    Vec4 hoverColor;    // Color when hovered
    Vec4 activeColor;   // Color when clicked
} UI_Style;


typedef struct UI_Element_t{
    struct UI_Element_t* parent;
    struct UI_Element_t* children;

    Transform2D transform;

    UI_Style style;
    u32 order;             // Render order priority

    

    struct{
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;
        u32 indicesCount;
        bool needsUpdate;  
    }renderer;

}UI_Element;

typedef struct UI_Manager_t{
    UI_Element root;
    f32 pixelsPerPoint;
    bool visible;
}UI_Manager;

#endif //UI_TYPES_H