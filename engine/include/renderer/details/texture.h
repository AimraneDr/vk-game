#ifndef TEXTURE_H
#define TEXTURE_H

#include "renderer/render_types.h"

void createTextureImage(VkPhysicalDevice gpu, VkDevice device, VkCommandPool cmdPool, VkQueue queue, u16* outMipLevels, VkImage* outTextureImage, VkDeviceMemory* outTextureMemory);
void createTextureImageView(VkDevice device, VkImage textureImage, u16 mipLevels, VkImageView* outTextureImageView);
void createTextureImageSampler(VkPhysicalDevice gpu, VkDevice device, u16 mipLevels, VkSampler* outTextureSampler);
void destroyTextureImage(VkDevice device, VkImage textureImage, VkDeviceMemory textureImageMemory);
void destroyTextureImageView(VkDevice device, VkImageView textureImageView);
void destroyTextureSampler(VkDevice device, VkSampler sampler);

#endif //TEXTURE_H