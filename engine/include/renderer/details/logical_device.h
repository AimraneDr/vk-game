#ifndef LOGICAL_DEVICE_H
#define LOGICAL_DEVICE_H

#include "renderer/render_types.h"

void createLogicalDevice(const VkPhysicalDevice device, const VkSurfaceKHR surface, VkDevice* out_device, VkQueue* out_graphics_queue, VkQueue* out_present_queue);
void destroyLogicalDevice(VkDevice lDevice);

#endif //LOGICAL_DEVICE_H