#ifndef COMMAND_POOL_H
#define COMMAND_POOL_H

#include "engine/renderer/render_types.h"

void createCommandPool(VkPhysicalDevice gpu, VkDevice device, VkSurfaceKHR surface, VkCommandPool *out);
void destroyCommandPool(VkDevice device, VkCommandPool commandPool);

#endif //COMMAND_POOL_H