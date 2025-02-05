#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include "renderer/render_types.h"
#include "meshTypes.h"

void createVertexBuffer(VkPhysicalDevice gpu, VkDevice device, VkQueue queue, VkCommandPool cmdPool, u32 verticesCount, Vertex* vertices, VkBuffer* outBuff, VkDeviceMemory* outMem);
void destroyVertexBuffer(VkDevice device, VkBuffer vertexBuffer, VkDeviceMemory mem);

#endif //VERTEX_BUFFER_H