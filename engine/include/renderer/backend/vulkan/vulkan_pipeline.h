#ifndef RENDERER_BACKEND_VULKAN_PIPELINE_H
#define RENDERER_BACKEND_VULKAN_PIPELINE_H

#include "./vulkan_types.h"
#include "renderer/render_types.h"

void createVulkanPipeline(
    VkDevice device, 
    VkExtent2D extent, VkSampleCountFlagBits msaaSamples, 
    VkRenderPass renderpass, Pipeline* pipeline, u8 index, const PipelineConfig* config);

void destroyVulkanPipeline(VkDevice device, VkPipeline pipeline, VkPipelineLayout layout);
#endif //