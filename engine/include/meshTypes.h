#ifndef MESH_TYPES_H
#define MESH_TYPES_H

#include "data_types.h"
#include <math/mathTypes.h>

typedef struct Vertex2D_t {
    Vec2 pos;
    Vec2 texCoord;
} Vertex2D;

typedef struct Vertex_t {
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