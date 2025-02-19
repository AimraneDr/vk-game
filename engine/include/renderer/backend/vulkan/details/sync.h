#ifndef RENDERER_BACKEND_VULKAN_DETAILS_SYNC_H
#define RENDERER_BACKEND_VULKAN_DETAILS_SYNC_H

#include "./../vulkan_types.h"

void createVulkanSyncObjects(VkDevice device, SyncObj* syncObjs);
void destroyVulkanSyncObjects(VkDevice device, SyncObj* syncObjs);

#endif //SYNC_H