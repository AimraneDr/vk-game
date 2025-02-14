#ifndef RENDERER_BACKEND_VULKAN_QUEUE_H
#define RENDERER_BACKEND_VULKAN_QUEUE_H

#include "renderer/backend/vulkan/vulkan_types.h"

QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device, const VkSurfaceKHR surface);
void freeQueeFamiliesInfoStruct(QueueFamilyIndices q);

#endif //RENDERER_BACKEND_VULKAN_QUEUE_H
