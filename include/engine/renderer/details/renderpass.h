#ifndef RENDERPASS_H
#define RENDERPASS_H

#include "engine/renderer/render_types.h"

void createRenderPass(VkPhysicalDevice gpu, VkDevice device, VkFormat swapchain_image_format, VkRenderPass* out_render_pass);
void destroyRenderPass(VkDevice device, VkRenderPass render_pass);

#endif //RENDERPASS_H