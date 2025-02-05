#ifndef BUFFER_H
#define BUFFER_H

#include "engine/renderer/render_types.h"

void createBuffer(VkPhysicalDevice gpu, VkDevice device, u32 size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memProperties, VkBuffer* outBuff, VkDeviceMemory* buffMem);
void destroyBuffer(VkDevice device, VkBuffer buff, VkDeviceMemory mem);

void copyBuffer(VkDevice device,VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool cmdPool, VkQueue queue);

#endif //BUFFER_H