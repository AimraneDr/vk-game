#include "components/transform.h"

#include <math/mat.h>
#include <math/trigonometry.h>
#include <math/vec3.h>

void transform_update(Transform* t, Mat4* parentSpace){
    t->mat = mat4_mul(
        mat4_scaling(vec3_mul(t->scale, vec3_new(1,-1,1))),
        mat4_mul(
            mat4_rotation(deg_to_rad(t->rotation.x), vec3_up()),
            mat4_translation(t->position)
        )
    );
    t->__local_mat = parentSpace ? mat4_mul(t->mat, *parentSpace) : mat4_identity();
}

void transform2D_update(Transform2D* t, Mat4* parentSpace) {
    Mat4 translation = mat4_translation(vec3_fromVec2(t->position, 0.f));
    Mat4 rotation = mat4_rotation(deg_to_rad(t->rotation), vec3_forward());
    Mat4 scaling = mat4_scaling(vec3_fromVec2(t->scale, 1.f));

    // t->mat = mat3_mul(translation, mat3_mul(rotation, scaling));
    t->mat = mat4_mul(scaling, mat4_mul(rotation, translation));
    t->__local_mat = parentSpace ? *parentSpace : mat4_identity();
}