#ifndef RENDERER_BACKEND_VULKAN_LOGICAL_DEVICE_H
#define RENDERER_BACKEND_VULKAN_LOGICAL_DEVICE_H

#include "renderer/backend/vulkan/vulkan_types.h"

void createLogicalDevice(const VkPhysicalDevice device, const VkSurfaceKHR surface, VkDevice* out_device, VkQueue* out_graphics_queue, VkQueue* out_present_queue);
void destroyLogicalDevice(VkDevice lDevice);

#endif //RENDERER_BACKEND_VULKAN_LOGICAL_DEVICE_H