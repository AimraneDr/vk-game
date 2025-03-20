#pragma once

#include "render_types.h"
#include "meshTypes.h"

void createMeshData(u64 vertices_count, Vertex* vertices, u64 indices_count, u32* indices, MeshData** out);
void destroyMeshData(MeshData** mesh);