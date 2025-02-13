#include "components/transform.h"

#include <math/mat.h>

void transform2D_update(Transform2D* t) {
    Mat3 translation = mat3_translation(t->position);
    Mat3 rotation = mat3_rotation(t->rotation, t->pivot);
    Mat3 scaling = mat3_scaling(t->scale);
    t->mat = mat3_mul(scaling, mat3_mul(rotation, translation));
}