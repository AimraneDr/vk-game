#ifndef IMAGE_H
#define IMAGE_H

#include "engine/renderer/render_types.h"

void createImage(
    VkPhysicalDevice gpu, VkDevice device,
    u32 width, u32 height, 
    VkFormat format, VkImageTiling tiling, 
    VkImageUsageFlags usage, 
    VkMemoryPropertyFlags properties,
    VkImage* outImage, VkDeviceMemory* outMem);
void destroyImage(VkDevice device, VkImage image, VkDeviceMemory imageMemory);

void transitionImageLayout(VkDevice device, VkCommandPool cmdPool, VkQueue queue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
void copyBufferToImage(VkDevice device, VkCommandPool cmdPool, VkQueue queue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

#endif //IMAGE_H