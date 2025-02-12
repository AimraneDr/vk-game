#ifndef UI_TYPES_H
#define UI_TYPES_H

#include "data_types.h"
#include <math/mathTypes.h>

typedef struct UI_Element_t{
    struct UI_Element* parent;
    struct UI_Element** children;
    u32 childrenCount;
}UI_Element;

typedef struct UI_Manager_t{
    UI_Element root;
    f32 pixelsPerPoint;
    bool visible;
}UI_Manager;

#endif //UI_TYPES_H