#include "components/camera.h"

#include "core/debugger.h"

#include <math/mat.h>
#include <math/trigonometry.h>
#include <math/vec3.h>

void camera_updateViewMat(Camera* camera){
    Vec3 radRot = vec3_new(
        deg_to_rad(camera->transform.rotation.x),
        deg_to_rad(camera->transform.rotation.y),
        deg_to_rad(camera->transform.rotation.z)
    );
    camera->view = mat4_viewYXZ(
        camera->transform.position, radRot); 
}

void camera_updateProjectionMat(Camera* camera, Vec2 frameExtent){
    f32 aspect = (f32)frameExtent.x / (f32)frameExtent.y;
    if (camera->useOrthographic){
        f32 orthoHeight = camera->orthographicSize;
        f32 orthoWidth = orthoHeight * aspect;
    
        camera->projection = mat4_orthographic(
            -orthoWidth,   // left
            orthoWidth,    // right
            -orthoHeight,  // bottom
            orthoHeight,   // top
            camera->nearPlane,
            camera->farPlane
        );
    }else{
        camera->projection = mat4_perspective(
            deg_to_rad(camera->fieldOfView),       // 45 degrees in radians
            aspect,
            camera->nearPlane,
            camera->farPlane
        );
    }
}