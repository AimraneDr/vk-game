#ifndef RENDERER_BACKEND_VULKAN_RESOURCE_COLOR_H
#define RENDERER_BACKEND_VULKAN_RESOURCE_COLOR_H

#include "./../../vulkan_types.h"

void createVulkanColorResources(
    VkPhysicalDevice gpu, VkDevice device, 
    VkCommandPool cmdPool, VkQueue queue,
    VkExtent2D swapChainExtent, VkSampleCountFlagBits msaaSamples,
    VkFormat colorFormat, 
    VkImage* outColorImage, 
    VkDeviceMemory* outColorImageMemory, VkImageView* outColorImageView);

void destroyVulkanColorResources(VkDevice device, VkImage colorImage, VkDeviceMemory colorImageMemory, VkImageView colorImageView);

#endif //RENDERER_BACKEND_VULKAN_RESOURCE_COLOR_H