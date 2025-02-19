#ifndef RENDERER_BACKEND_VULKAN_RESOURCE_DEPTH_H
#define RENDERER_BACKEND_VULKAN_RESOURCE_DEPTH_H

#include "./../../vulkan_types.h"

void createVulkanDepthResources(
    VkPhysicalDevice gpu, VkDevice device, 
    VkCommandPool cmdPool, VkQueue queue,
    VkExtent2D swapChainExtent,  VkSampleCountFlagBits msaaSamples,
    VkImage* outDepthImage, 
    VkDeviceMemory* outDepthImageMemory, VkImageView* outDepthImageView);

void destroyVulkanDepthResources(VkDevice device, VkImage depthImage, VkDeviceMemory depthImageMemory, VkImageView depthImageView);

VkFormat findDepthFormat(VkPhysicalDevice gpu);

#endif //RENDERER_BACKEND_VULKAN_RESOURCE_DEPTH_H