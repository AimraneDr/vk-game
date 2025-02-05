#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include "engine/renderer/render_types.h"

void createDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout* out);
void destroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout);

void createDescriptorPool(VkDevice device, VkDescriptorPool* out);
void destroyDescriptorPool(VkDevice device, VkDescriptorPool pool);

void createDescriptorSets(VkDevice device, VkDescriptorSetLayout setLayout, VkDescriptorPool pool, VkBuffer* uniformBuffers, VkImageView textureImageView, VkSampler textureSampler, VkDescriptorSet* descriptorSets);

#endif