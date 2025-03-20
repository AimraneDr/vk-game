#ifndef CAMERA_COMPONENT_H
#define CAMERA_COMPONENT_H

#include "data_types.h"
#include "engine_defines.h"
#include "./transform.h"

typedef struct Camera_Component_t{
    Transform transform;
    f32 fieldOfView;
    f32 farPlane;
    f32 nearPlane;
    f32 orthographicSize;

    //affects ui
    f32 pixelsPerPoint;
    Vec2i viewRect;

    bool useOrthographic;
    Mat4 view, projection;
}Camera;


void camera_updateViewMat(Camera* camera);
void camera_updateProjectionMat(Camera* camera, Vec2 frameExtent);

API Vec3 camera_forward(Camera* c);
API Vec3 camera_up(Camera* c);
API Vec3 camera_right(Camera* c);

#endif //CAMERA_COMPONENT_H