#ifndef RENDERER_BACKEND_VULKAN_RESOURCE_BUFFER_H
#define RENDERER_BACKEND_VULKAN_RESOURCE_BUFFER_H

#include "./../../vulkan_types.h"

void createVulkanBuffer(
    VkPhysicalDevice gpu, 
    VkDevice device, 
    VkDeviceSize size, 
    VkBufferUsageFlags usageFlags, 
    VkMemoryPropertyFlags memProperties, 
    VkBuffer* outBuff, 
    VkDeviceMemory* buffMem);
void destroyVulkanBuffer(VkDevice device, VkBuffer buff, VkDeviceMemory mem);

void copyBuffer(VkDevice device,VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool cmdPool, VkQueue queue);

#endif //RENDERER_BACKEND_VULKAN_RESOURCE_BUFFER_H