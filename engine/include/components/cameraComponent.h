#ifndef CAMERA_COMPONENT_H
#define CAMERA_COMPONENT_H

#include "data_types.h"
#include "./transformComponent.h"

typedef struct Camera_Component_t{
    Transform_Component transform;
    f32 fieldOfView;
    f32 farPlane;
    f32 nearPlane;
    f32 orthographicSize;

    bool useOrthographic;
    Mat4 view, projection;
}Camera_Component;


void camera_updateViewMat(Camera_Component* camera);
void camera_updateProjectionMat(Camera_Component* camera, Vec2 frameExtent);


#endif //CAMERA_COMPONENT_H