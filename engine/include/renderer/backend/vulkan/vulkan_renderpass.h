#ifndef RENDERER_BACKEND_VULKAN_RENDERPASS
#define RENDERER_BACKEND_VULKAN_RENDERPASS

#include "./vulkan_types.h"

void createVulkanRenderPass(VkPhysicalDevice gpu, VkDevice device, VkFormat swapchain_image_format, const RenderpassInitConfig* config, VkRenderPass* out_render_pass);
void destroyVulkanRenderPass(VulkanContext* c, RenderPass* render_pass);

#endif //RENDERER_BACKEND_VULKAN_RENDERPASS