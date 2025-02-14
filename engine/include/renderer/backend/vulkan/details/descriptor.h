#ifndef RENDERER_BACKEND_VULKAN_DESCRIPTOR_H
#define RENDERER_BACKEND_VULKAN_DESCRIPTOR_H

#include "renderer/backend/vulkan/vulkan_types.h"
#include "renderer/render_types.h"

void createVulkanDescriptorPool(VkDevice device, const DescriptorSetConfig* config, VkDescriptorPool* out);
void destroyVulkanDescriptorPool(VkDevice device, VkDescriptorPool pool);

void createVulkanDescriptorSetLayout(VkDevice device, const DescriptorSetConfig* configs, u8 setsCount, VkDescriptorSetLayout* out);
void destroyVulkanDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout layout);
#endif //RENDERER_BACKEND_VULKAN_DESCRIPTOR_POOL_LAYOUT_H