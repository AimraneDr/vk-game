#include "components/transform.h"

#include <math/mat.h>
#include <math/trigonometry.h>
#include <math/vec3.h>

void transform_update(Transform* t, Mat4* parentSpace){
    Mat4 translation = mat4_translation(t->position);
    Mat4 rotation = mat4_rotation_euler(t->rotation);
    Mat4 scaling = mat4_scaling(vec3_mul(t->scale, vec3_new(1,-1,1)));

    t->mat = mat4_mul(scaling, mat4_mul(rotation, translation));
    t->__local_mat = parentSpace ? *parentSpace : mat4_identity();
}

void transform2D_update(Transform2D* t, Mat4* parentSpace) {
    Mat4 translation = mat4_translation(vec3_fromVec2(t->position, 0));
    Mat4 rotation = mat4_rotation(deg_to_rad(t->rotation), vec3_forward());
    Mat4 scaling = mat4_scaling(vec3_fromVec2(t->scale, 1.f));

    // t->mat = mat4_mul(translation, mat4_mul(rotation, scaling));
    t->mat = mat4_mul(scaling, mat4_mul(rotation, translation));
    t->__local_mat = parentSpace ? *parentSpace : mat4_identity();
}

Vec3 transform_forward(Transform* t){
    return mat4_forward(t->mat);
}
Vec3 transform_up(Transform* t){
    return mat4_up(t->mat);
}
Vec3 transform_right(Transform* t){
    return mat4_right(t->mat);
}