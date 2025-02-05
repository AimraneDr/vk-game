#ifndef CAMERA_COMPONENT_H
#define CAMERA_COMPONENT_H

#include "data_types.h"
#include "./transformComponent.h"

typedef struct Camera_Component_t{
    Transform_Component transform;
    u32 fiealdOfView;
    u32 farPlane;
    u32 nearPlane;

    Mat4 view, projection;
}Camera_Component;


void camera_updateViewMat(Camera_Component* camera);
void camera_updateProjectionMat(Camera_Component* camera);


#endif //CAMERA_COMPONENT_H