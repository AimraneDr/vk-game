#include "components/transform.h"

#include <math/mat.h>
#include <math/trigonometry.h>
#include <math/vec3.h>

void transform2D_update(Transform2D* t) {
    Mat4 translation = mat4_translation(vec3_fromVec2(t->position, 0.f));
    Mat4 rotation = mat4_rotation(deg_to_rad(t->rotation), VEC3_FORWARD);
    Mat4 scaling = mat4_scaling(vec3_fromVec2(t->scale, 1.f));

    // t->mat = mat3_mul(translation, mat3_mul(rotation, scaling));
    t->mat = mat4_mul(scaling, mat4_mul(rotation, translation));
}