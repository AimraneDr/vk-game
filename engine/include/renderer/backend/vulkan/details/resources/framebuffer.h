#ifndef RENDERER_BACKEND_VULKAN_RESOURCE_FRAMEBUFFER_H
#define RENDERER_BACKEND_VULKAN_RESOURCE_FRAMEBUFFER_H

#include "./../../vulkan_types.h"

void createVulkanFramebuffers(
    VkDevice device, 
    VkRenderPass render_pass, 
    VkImageView *swapchain_image_views, 
    VkImageView colorImageView,
    VkImageView depthImageView, 
    u32 swapchain_image_count, VkExtent2D swapchain_extent, 
    VkFramebuffer* *out_framebuffers);
    
void destroyVulkanFramebuffers(VkDevice device, VkFramebuffer *framebuffers, u32 framebuffers_count);

#endif //RENDERER_BACKEND_VULKAN_RESOURCE_FRAMEBUFFER_H