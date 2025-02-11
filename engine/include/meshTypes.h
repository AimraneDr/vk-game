#ifndef MESH_TYPES_H
#define MESH_TYPES_H

#include "data_types.h"
#include <math/mathTypes.h>

typedef struct Vertex {
    Vec3 pos;
    Vec3 norm;
    Vec2 texCoord;
}Vertex;

typedef struct Model_t{
    u32 id;
    Vertex* vertices;
    u32* indices;
    u32 vertex_count;
    u32 index_count;
}Model;

#endif //MESH_TYPES_H