#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "engine/renderer/render_types.h"

void createFramebuffers(VkDevice device, VkRenderPass render_pass, VkImageView *swapchain_image_views, VkImageView depthImageView, u32 swapchain_image_count, VkExtent2D swapchain_extent, VkFramebuffer* *out_framebuffers);
void destroyFramebuffers(VkDevice device, VkFramebuffer *framebuffers, u32 framebuffers_count);

#endif //FRAMEBUFFER_H