#ifndef PHYSICAL_DEVICE_H
#define PHYSICAL_DEVICE_H

#include "renderer/render_types.h"

void selectPhysicalDevice(const VkInstance instance, const VkSurfaceKHR surface, VkPhysicalDevice *out,  VkSampleCountFlagBits* outMsaaSamples);
bool checkDeviceExtensionsSupport(VkPhysicalDevice device);

VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice gpu);
u32 findMemoryType(VkPhysicalDevice gpu, u32 typeFilter, VkMemoryPropertyFlags properties);

#endif //PHYSICAL_DEVICE_H