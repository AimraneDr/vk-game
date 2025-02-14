#ifndef RENDERER_BACKEND_VULKAN_PHYSICAL_DEVICE_H
#define RENDERER_BACKEND_VULKAN_PHYSICAL_DEVICE_H

#include "renderer/backend/vulkan/vulkan_types.h"

void selectPhysicalDevice(const VkInstance instance, const VkSurfaceKHR surface, VkPhysicalDevice *out,  VkSampleCountFlagBits* outMaxMsaaSamples);
bool checkDeviceExtensionsSupport(VkPhysicalDevice device);

VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice gpu);
u32 findMemoryType(VkPhysicalDevice gpu, u32 typeFilter, VkMemoryPropertyFlags properties);

#endif //RENDERER_BACKEND_VULKAN_PHYSICAL_DEVICE_H