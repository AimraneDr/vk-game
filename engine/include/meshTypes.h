#ifndef MESH_TYPES_H
#define MESH_TYPES_H

#include <math/mathTypes.h>

typedef struct Vertex {
    Vec3 pos;
    Vec3 norm;
    Vec4 color;
    Vec2 texCoord;
}Vertex;

#endif //MESH_TYPES_H