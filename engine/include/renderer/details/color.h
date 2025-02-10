#ifndef COLOR_H
#define COLOR_H

#include "renderer/render_types.h"

void createColorResources(
    VkPhysicalDevice gpu, VkDevice device, 
    VkCommandPool cmdPool, VkQueue queue,
    VkExtent2D swapChainExtent, VkSampleCountFlagBits msaaSamples,
    VkFormat colorFormat, 
    VkImage* outColorImage, 
    VkDeviceMemory* outColorImageMemory, VkImageView* outColorImageView);

void destroyColorResources(VkDevice device, VkImage colorImage, VkDeviceMemory colorImageMemory, VkImageView colorImageView);

#endif //COLOR_H