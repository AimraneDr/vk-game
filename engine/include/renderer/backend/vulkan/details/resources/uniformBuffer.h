#ifndef RENDERER_BACKEND_VULKAN_RESOURCE_UNIFORM_BUFFER_H
#define RENDERER_BACKEND_VULKAN_RESOURCE_UNIFORM_BUFFER_H

#include "./../../vulkan_types.h"

void createVulkanUniformBuffers(
    VkPhysicalDevice gpu, 
    VkDevice device,
    DescriptorSetConfig* descriptorConfig,
    PipelineResources* resources
);
#endif //RENDERER_BACKEND_VULKAN_RESOURCE_UNIFORM_BUFFER_H