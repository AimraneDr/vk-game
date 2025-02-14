#ifndef RENDERER_BACKEND_VULKAN_RENDERPASS
#define RENDERER_BACKEND_VULKAN_RENDERPASS

#include "./vulkan_types.h"

void createVulkanRenderPass(VkPhysicalDevice gpu, VkDevice device, VkFormat swapchain_image_format, VkSampleCountFlagBits msaaSamples, VkRenderPass* out_render_pass);
void destroyVulkanRenderPass(VkDevice device, VkRenderPass render_pass);

#endif //RENDERER_BACKEND_VULKAN_RENDERPASS