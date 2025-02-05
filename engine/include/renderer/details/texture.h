#ifndef TEXTURE_H
#define TEXTURE_H

#include "renderer/render_types.h"

void createTextureImage(VkPhysicalDevice gpu, VkDevice device, VkCommandPool cmdPool, VkQueue queue, VkImage* outTextureImage, VkDeviceMemory* outTextureMemory);
void createTextureImageView(VkDevice device, VkImage textureImage, VkImageView* outTextureImageView);
void createTextureImageSampler(VkPhysicalDevice gpu, VkDevice device, VkSampler* outTextureSampler);
void destroyTextureImage(VkDevice device, VkImage textureImage, VkDeviceMemory textureImageMemory);
void destroyTextureImageView(VkDevice device, VkImageView textureImageView);
void destroyTextureSampler(VkDevice device, VkSampler sampler);

#endif //TEXTURE_H