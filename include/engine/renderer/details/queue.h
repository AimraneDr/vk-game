#ifndef QUEUE_H
#define QUEUE_H

#include "engine/renderer/render_types.h"

QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device, const VkSurfaceKHR surface);
void freeQueeFamiliesInfoStruct(QueueFamilyIndices q);

#endif //QUEUE_H
