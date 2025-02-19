#ifndef RENDERER_BACKEND_VULKAN_CONTEXT_H
#define RENDERER_BACKEND_VULKAN_CONTEXT_H

#include "./vulkan_types.h"
#include "platform/platform_types.h"

void createVulkanRenderingContext(VulkanContext* c, PlatformState* p, VkSampleCountFlagBits* outMssaSamples);
void destroyVulkanRenderingContext(VulkanContext* c);

#endif //RENDERER_BACKEND_VULKAN_CONTEXT_H