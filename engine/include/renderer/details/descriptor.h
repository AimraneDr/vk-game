#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include "renderer/render_types.h"

void PBR_createDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout* out);
void UI_createDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout* out);
void destroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout);

void PBR_createDescriptorPool(VkDevice device, VkDescriptorPool* out);
void UI_createDescriptorPool(VkDevice device, VkDescriptorPool* out);
void destroyDescriptorPool(VkDevice device, VkDescriptorPool pool);

void PBR_createDescriptorSets(VkDevice device, VkDescriptorSetLayout setLayout, VkDescriptorPool pool, VkBuffer* uniformBuffers, VkImageView textureImageView, VkSampler textureSampler, VkDescriptorSet* descriptorSets);
void UI_createDescriptorSets(
    VkDevice device, 
    VkDescriptorSetLayout setLayout, 
    VkDescriptorPool pool, 
    VkBuffer* globalUniformBuffers, 
    VkBuffer* elementUniformBuffers, 
    VkDeviceSize elementAlignedUboSize,
    VkImageView textureImageView, 
    VkSampler textureSampler, 
    VkDescriptorSet* outDescriptorSets);

#endif