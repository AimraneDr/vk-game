#ifndef RENDERER_BACKEND_VULKAN_RESOURCE_TEXTURE_H
#define RENDERER_BACKEND_VULKAN_RESOURCE_TEXTURE_H

#include "./../../vulkan_types.h"

void createVulkanTextures(VulkanContext* context, VkCommandPool cmdPool, DescriptorSetConfig* config, Texture** outTextures, u16* outCount);

void createTextureImage(VkPhysicalDevice gpu, VkDevice device, VkCommandPool cmdPool, VkQueue queue, String path, u16* outMipLevels, VkImage* outTextureImage, VkDeviceMemory* outTextureMemory);
void createTextureImageView(VkDevice device, VkImage textureImage, u16 mipLevels, VkImageView* outTextureImageView);
void createTextureImageSampler(VkPhysicalDevice gpu, VkDevice device, u16 mipLevels, VkSampler* outTextureSampler);
void destroyTextureImage(VkDevice device, VkImage textureImage, VkDeviceMemory textureImageMemory);
void destroyTextureImageView(VkDevice device, VkImageView textureImageView);
void destroyTextureSampler(VkDevice device, VkSampler sampler);

#endif //RENDERER_BACKEND_VULKAN_RESOURCE_TEXTURE_H