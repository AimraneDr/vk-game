#ifndef RENDERER_BACKEND_VULKAN_COMMAND_H
#define RENDERER_BACKEND_VULKAN_COMMAND_H

#include "./vulkan_types.h"

void createVulkanCommandPool(VkPhysicalDevice gpu, VkDevice device, VkSurfaceKHR surface, VkCommandPool *out);
void destroyVulkanCommandPool(VkDevice device, VkCommandPool commandPool);

void createVulkanCommandBuffer(VkDevice device, VkCommandPool pool, VkCommandBuffer* out);
void destroyVulkanCommandBuffer(VkDevice device, VkCommandPool pool, VkCommandBuffer* buff);

VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool cmdPool);
void endSingleTimeCommands(VkDevice device, VkCommandPool cmdPool, VkQueue queue,  VkCommandBuffer* cmdBuffer);

void recordCommandBuffer();

#endif //RENDERER_BACKEND_VULKAN_COMMAND_H