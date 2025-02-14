#ifndef RENDERER_BACKEND_VULKAN_CONTEXT_H
#define RENDERER_BACKEND_VULKAN_CONTEXT_H

#include "platform/platform_types.h"
#include "renderer/render_types.h"

void createVulkanRenderingContext(VulkanContext* c, PlatformState* p, VkSampleCountFlagBits* outMssaSamples);
void destroyVulkanRenderingContext(VulkanContext* c);

#endif //RENDERER_BACKEND_VULKAN_CONTEXT_H