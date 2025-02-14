#ifndef RENDERER_BACKEND_VULKAN_SWAPCHAIN_OBJ_H
#define RENDERER_BACKEND_VULKAN_SWAPCHAIN_OBJ_H

#include "renderer/backend/vulkan/vulkan_types.h"

void createVulkanSwapchainObj(VulkanContext* context, u32 width, u32 height, SwapchainObj* out);

void destroyVulkanSwapchainObj(VulkanContext* context, SwapchainObj* swapchain);

#endif //RENDERER_BACKEND_VULKAN_SWAPCHAIN_OBJ_H