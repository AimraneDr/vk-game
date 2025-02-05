#include "components/cameraComponent.h"

#include <math/mat.h>
#include <math/trigonometry.h>

void camera_updateViewMat(Camera_Component* camera){
    camera->view = mat4_viewYXZ(
        camera->transform.position, camera->transform.rotation); 
}

void camera_updateProjectionMat(Camera_Component* camera){
    camera->projection = mat4_perspective(
        deg_to_rad(camera->fiealdOfView),       // 45 degrees in radians
        camera->transform.scale.x / camera->transform.scale.y,
        camera->nearPlane,
        camera->farPlane
    );
}