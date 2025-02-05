#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H

#include <math/mathTypes.h>

typedef struct Transform_Component_t{
    Vec3 position;
    Vec3 rotation;
    Vec3 scale;
}Transform_Component;

#endif //TRANSFORM_COMPONENT_H