#include "components/transform.h"

#include <math/mat.h>
#include <math/trigonometry.h>
#include <math/vec2.h>

void transform2D_update(Transform2D* t) {
    Mat3 translation = mat3_translation(t->position);
    Mat3 rotation = mat3_rotation(deg_to_rad(t->rotation), VEC2_ZERO);
    Mat3 scaling = mat3_scaling(t->scale);

    // t->mat = mat3_mul(translation, mat3_mul(rotation, scaling));
    t->mat = mat3_mul(scaling, mat3_mul(rotation, translation));
}