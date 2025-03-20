#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H

#include "data_types.h"
#include "engine_defines.h"
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

API Vec3 transform_forward(Transform* t);
API Vec3 transform_up(Transform* t);
API Vec3 transform_right(Transform* t);

#endif //TRANSFORM_COMPONENT_H