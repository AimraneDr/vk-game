#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H

#include "data_types.h"
#include <math/mathTypes.h>
#include "ecs/ecs_types.h"

typedef struct Transform_Component_t{
    Vec3 position;
    Vec3 rotation;
    Vec3 scale;
    Mat4 mat;
    Mat4 __local_mat;
}Transform;

typedef struct Transform2D_Component_t{
    Vec2 position;
    Vec2 scale;
    Mat4 mat;
    Mat4 __local_mat;
    f32 rotation;
}Transform2D;

void transform_update(Transform* t, Mat4* parentSpace);
void transform2D_update(Transform2D* t, Mat4* parentSpace);

#endif //TRANSFORM_COMPONENT_H