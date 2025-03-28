#ifndef DEPTH_H
#define DEPTH_H

#include "renderer/render_types.h"

void createDepthResources(
    VkPhysicalDevice gpu, VkDevice device, 
    VkCommandPool cmdPool, VkQueue queue,
    VkExtent2D swapChainExtent,  VkSampleCountFlagBits msaaSamples,
    VkImage* outDepthImage, 
    VkDeviceMemory* outDepthImageMemory, VkImageView* outDepthImageView);

void destroyDepthResources(VkDevice device, VkImage depthImage, VkDeviceMemory depthImageMemory, VkImageView depthImageView);

VkFormat findDepthFormat(VkPhysicalDevice gpu);

#endif //DEPTH_H