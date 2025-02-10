#ifndef RENDERPASS_H
#define RENDERPASS_H

#include "renderer/render_types.h"

void createRenderPass(VkPhysicalDevice gpu, VkDevice device, VkFormat swapchain_image_format, VkSampleCountFlagBits msaaSamples, VkRenderPass* out_render_pass);
void destroyRenderPass(VkDevice device, VkRenderPass render_pass);

#endif //RENDERPASS_H