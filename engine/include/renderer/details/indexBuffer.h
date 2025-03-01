#ifndef INDEX_BUFFER_H
#define INDEX_BUFFER_H

#include "renderer/render_types.h"

void createIndexBuffer(VkPhysicalDevice gpu, VkDevice device, VkQueue queue, VkCommandPool cmdPool, u32 indicesCount, const u32* indices, VkBuffer* outBuff, VkDeviceMemory* outMem);
void destroyIndexBuffer(VkDevice device, VkBuffer vertexBuffer, VkDeviceMemory mem);

#endif //INDEX_BUFFER_H