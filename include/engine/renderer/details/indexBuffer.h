#ifndef INDEX_BUFFER_H
#define INDEX_BUFFER_H

#include "engine/renderer/render_types.h"

void createIndexBuffer(VkPhysicalDevice gpu, VkDevice device, VkQueue queue, VkCommandPool cmdPool, u32 indicesCount, u16* indices, VkBuffer* outBuff, VkDeviceMemory* outMem);
void destroyIndexBuffer(VkDevice device, VkBuffer vertexBuffer, VkDeviceMemory mem);

#endif //INDEX_BUFFER_H